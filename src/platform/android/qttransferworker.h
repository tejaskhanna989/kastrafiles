// ── Android: Qt twin of GioTransferWorker ────────────────────────────
//
// This header shadows services/giotransferworker.h on Android (via an
// include-path redirect in CMake). It exposes the *same* class name
// (GioTransferWorker) and the *same* public interface — the TransferItem
// struct, execute/cancel/pause/resume, and the progressUpdated /
// itemStarted / finished signals — but implements copy/move in pure Qt
// (QFile/QDir) instead of GIO, which does not exist on Android.
//
// Because the interface is identical, FileOperations::startGioTransfer()
// compiles and wires up unchanged; only the backing implementation swaps.

#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QList>
#include <QMutex>
#include <QString>
#include <QWaitCondition>

#include <atomic>

class GioTransferWorker : public QObject
{
    Q_OBJECT

public:
    struct TransferItem {
        QString sourcePath;
        QString targetPath;
        QString backupPath;
        bool overwrite = false;
    };

    explicit GioTransferWorker(QObject *parent = nullptr);
    ~GioTransferWorker() override;

    void execute(const QList<TransferItem> &items, bool moveOperation);
    void cancel();
    void pause();
    void resume();

signals:
    void progressUpdated(double progress, const QString &speed, const QString &eta);
    void itemStarted(const QString &sourcePath, const QString &targetPath);
    void finished(bool success, const QString &error);

private:
    qint64 scanTotalBytes(const QList<TransferItem> &items) const;
    qint64 scanPathBytes(const QString &path) const;
    bool copyRecursive(const QString &source, const QString &destination, QString *error);
    bool deleteRecursive(const QString &path, QString *error);
    bool transferItem(const TransferItem &item, bool moveOperation, QString *error);
    void checkpoint(); // honours cancel/pause; returns early if cancelled
    void emitProgress();

    std::atomic<bool> m_cancelled{false};
    std::atomic<bool> m_paused{false};
    QMutex m_mutex;
    QWaitCondition m_pauseCondition;

    qint64 m_totalBytes = 0;
    qint64 m_completedBytes = 0;
    qint64 m_currentItemBytes = 0;
    QElapsedTimer m_elapsed;
    qint64 m_lastEmitMs = 0;
    double m_lastEmittedProgress = -1.0;
};
