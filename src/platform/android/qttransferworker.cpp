// ── Android: Qt implementation of GioTransferWorker ──────────────────
// See qttransferworker.h for why this class keeps the GioTransferWorker
// name. Copy/move are done with QFile/QDir so they work on Android's
// shared storage (which the app reaches as ordinary filesystem paths once
// it holds the "All files access" permission).

#include "platform/android/qttransferworker.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <cstring>

namespace {

QString ioErrorText(const QString &path)
{
    return QStringLiteral("Could not access %1").arg(QFileInfo(path).fileName());
}

} // namespace

GioTransferWorker::GioTransferWorker(QObject *parent)
    : QObject(parent)
{
}

GioTransferWorker::~GioTransferWorker() = default;

qint64 GioTransferWorker::scanPathBytes(const QString &path) const
{
    const QFileInfo info(path);
    if (!info.exists())
        return 0;
    if (info.isDir()) {
        qint64 total = 0;
        const QDir dir(path);
        const auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot
                                           | QDir::Hidden | QDir::System);
        for (const QString &entry : entries)
            total += scanPathBytes(dir.filePath(entry));
        return total;
    }
    return info.size();
}

qint64 GioTransferWorker::scanTotalBytes(const QList<TransferItem> &items) const
{
    qint64 total = 0;
    for (const TransferItem &item : items) {
        total += scanPathBytes(item.sourcePath);
        // Overwriting an existing target frees the old bytes, but counting
        // both keeps progress conservative (never over-reports 100% early).
        if (item.overwrite && QFileInfo::exists(item.targetPath))
            total += scanPathBytes(item.targetPath);
    }
    return total;
}

void GioTransferWorker::checkpoint()
{
    if (m_cancelled.load())
        return;
    // Park while paused, waking on resume()/cancel().
    QMutexLocker locker(&m_mutex);
    while (m_paused.load() && !m_cancelled.load())
        m_pauseCondition.wait(&m_mutex);
}

void GioTransferWorker::emitProgress()
{
    const qint64 now = m_elapsed.elapsed();
    // Throttle to ~every 100ms, but always emit on completion.
    const bool timeToEmit = (now - m_lastEmitMs) >= 100;
    const double fraction = m_totalBytes > 0
        ? double(m_completedBytes) / double(m_totalBytes)
        : 0.0;
    const bool crossedToDone = fraction >= 1.0 && m_lastEmittedProgress < 1.0;
    if (!timeToEmit && !crossedToDone)
        return;

    m_lastEmitMs = now;
    m_lastEmittedProgress = fraction;

    const double seconds = now / 1000.0;
    const double bytesPerSec = seconds > 0.0 ? double(m_completedBytes) / seconds : 0.0;
    QString speedText;
    QString etaText;
    if (bytesPerSec > 0.0 && m_totalBytes > m_completedBytes) {
        const double kib = bytesPerSec / 1024.0;
        speedText = kib >= 1024.0
            ? QStringLiteral("%1 MB/s").arg(kib / 1024.0, 0, 'f', 1)
            : QStringLiteral("%1 KB/s").arg(kib, 0, 'f', 0);
        const qint64 remaining = m_totalBytes - m_completedBytes;
        const qint64 etaSecs = qint64(remaining / bytesPerSec);
        etaText = QStringLiteral("%1:%2")
                      .arg(etaSecs / 60)
                      .arg(etaSecs % 60, 2, 10, QLatin1Char('0'));
    }
    emit progressUpdated(fraction, speedText, etaText);
}

bool GioTransferWorker::copyRecursive(const QString &source, const QString &destination, QString *error)
{
    checkpoint();
    if (m_cancelled.load()) {
        if (error)
            *error = QStringLiteral("Transfer cancelled");
        return false;
    }

    const QFileInfo srcInfo(source);
    if (srcInfo.isDir()) {
        QDir().mkpath(destination);
        const QDir dir(source);
        const auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot
                                           | QDir::Hidden | QDir::System);
        for (const QString &entry : entries) {
            if (!copyRecursive(dir.filePath(entry), destination + QLatin1Char('/') + entry, error))
                return false;
        }
        return true;
    }

    // Regular file (or symlink): copy bytes in chunks so progress + pause
    // stay responsive on large files.
    QFile src(source);
    if (!src.open(QIODevice::ReadOnly)) {
        if (error)
            *error = ioErrorText(source);
        return false;
    }
    QFile dst(destination);
    if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error)
            *error = ioErrorText(destination);
        return false;
    }

    constexpr qint64 kChunk = 1024 * 256;
    while (!src.atEnd()) {
        checkpoint();
        if (m_cancelled.load()) {
            if (error)
                *error = QStringLiteral("Transfer cancelled");
            return false;
        }
        const QByteArray chunk = src.read(kChunk);
        if (chunk.isEmpty() && !src.atEnd()) {
            if (error)
                *error = ioErrorText(source);
            return false;
        }
        if (dst.write(chunk) != chunk.size()) {
            if (error)
                *error = ioErrorText(destination);
            return false;
        }
        m_completedBytes += chunk.size();
        m_currentItemBytes += chunk.size();
        emitProgress();
    }

    dst.close();
    src.close();
    // Preserve executable bits etc. where the filesystem allows it.
    dst.setPermissions(srcInfo.permissions());
    return true;
}

bool GioTransferWorker::deleteRecursive(const QString &path, QString *error)
{
    checkpoint();
    if (m_cancelled.load()) {
        if (error)
            *error = QStringLiteral("Transfer cancelled");
        return false;
    }

    const QFileInfo info(path);
    if (info.isDir()) {
        const QDir dir(path);
        const auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot
                                           | QDir::Hidden | QDir::System);
        for (const QString &entry : entries) {
            if (!deleteRecursive(dir.filePath(entry), error))
                return false;
        }
        if (!dir.rmdir(path)) {
            if (error)
                *error = ioErrorText(path);
            return false;
        }
        return true;
    }

    if (!QFile::remove(path)) {
        if (error)
            *error = ioErrorText(path);
        return false;
    }
    return true;
}

bool GioTransferWorker::transferItem(const TransferItem &item, bool moveOperation, QString *error)
{
    emit itemStarted(item.sourcePath, item.targetPath);

    // Resolve a non-colliding target when overwrite is not requested.
    QString target = item.targetPath;
    if (!item.overwrite && QFileInfo::exists(target)) {
        target = item.backupPath.isEmpty() ? target + QStringLiteral(".kastrafiles-copy")
                                           : item.backupPath;
    }

    if (!copyRecursive(item.sourcePath, target, error))
        return false;

    if (moveOperation) {
        // Only remove the source once the copy fully succeeded.
        if (!deleteRecursive(item.sourcePath, error))
            return false;
    }
    return true;
}

void GioTransferWorker::execute(const QList<TransferItem> &items, bool moveOperation)
{
    m_cancelled.store(false);
    m_paused.store(false);
    m_completedBytes = 0;
    m_currentItemBytes = 0;
    m_lastEmitMs = 0;
    m_lastEmittedProgress = -1.0;

    m_totalBytes = scanTotalBytes(items);
    m_elapsed.start();

    QString error;
    bool ok = true;
    for (const TransferItem &item : items) {
        if (m_cancelled.load()) {
            ok = false;
            error = QStringLiteral("Transfer cancelled");
            break;
        }
        if (!transferItem(item, moveOperation, &error)) {
            ok = false;
            break;
        }
    }

    if (ok && m_totalBytes > 0) {
        // Ensure a final 1.0 progress tick so the UI settles at 100%.
        m_completedBytes = m_totalBytes;
        m_lastEmittedProgress = -1.0;
        emitProgress();
    }

    emit finished(ok, error);
}

void GioTransferWorker::cancel()
{
    m_cancelled.store(true);
    QMutexLocker locker(&m_mutex);
    m_pauseCondition.wakeAll();
}

void GioTransferWorker::pause()
{
    m_paused.store(true);
}

void GioTransferWorker::resume()
{
    m_paused.store(false);
    QMutexLocker locker(&m_mutex);
    m_pauseCondition.wakeAll();
}
