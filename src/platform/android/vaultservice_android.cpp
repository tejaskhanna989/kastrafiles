// ── Android stub ─────────────────────────────────────────────────────
// VaultService on Android. The real vault encrypts files with AES-256-GCM
// (OpenSSL) and derives keys with argon2, then flips OS-level immutable
// bits — none of which exist on Android. This stub keeps the exact QML
// interface (header unchanged) so every `vault.*` call in Main.qml /
// ContextMenu.qml / LockPasswordDialog.qml stays valid, but the vault is
// inert: nothing is ever "locked", so all lock/unlock paths are no-ops and
// isLocked() always returns false. The UI therefore behaves as a plain
// file manager. CryptoEngine/VaultDatabase are never constructed, so their
// OpenSSL/argon2 dependencies are not linked on this platform.

#include "services/vaultservice.h"

VaultService::VaultService(const QString &configDir, QObject *parent)
    : QObject(parent)
    , m_crypto(nullptr)
    , m_db(nullptr)
    , m_configDir(configDir)
{
}

VaultService::~VaultService() = default;

bool VaultService::lockItem(const QString &path, const QString &password)
{
    Q_UNUSED(path)
    Q_UNUSED(password)
    m_lastError = QStringLiteral("The secure vault is not available on Android.");
    return false;
}

bool VaultService::lockItems(const QStringList &paths, const QString &password)
{
    Q_UNUSED(paths)
    Q_UNUSED(password)
    m_lastError = QStringLiteral("The secure vault is not available on Android.");
    return false;
}

bool VaultService::unlockItem(const QString &path, const QString &password)
{
    Q_UNUSED(path)
    Q_UNUSED(password)
    m_lastError = QStringLiteral("The secure vault is not available on Android.");
    return false;
}

bool VaultService::isLocked(const QString &path) const
{
    Q_UNUSED(path)
    // Nothing can ever be locked on Android, so nothing is blocked.
    return false;
}

bool VaultService::isSessionUnlocked(const QString &path) const
{
    Q_UNUSED(path)
    return false;
}

bool VaultService::changePassword(const QString &path, const QString &oldPassword, const QString &newPassword)
{
    Q_UNUSED(path)
    Q_UNUSED(oldPassword)
    Q_UNUSED(newPassword)
    m_lastError = QStringLiteral("The secure vault is not available on Android.");
    return false;
}

bool VaultService::sessionUnlockFolder(const QString &path, const QString &password)
{
    Q_UNUSED(path)
    Q_UNUSED(password)
    return false;
}

void VaultService::sessionRelockFolder(const QString &path)
{
    Q_UNUSED(path)
}

bool VaultService::sessionUnlockFile(const QString &path, const QString &password)
{
    Q_UNUSED(path)
    Q_UNUSED(password)
    return false;
}

bool VaultService::sessionOpenFile(const QString &path, const QString &password)
{
    Q_UNUSED(path)
    Q_UNUSED(password)
    return false;
}

bool VaultService::sessionOpenFileWith(const QString &path, const QString &desktopFile, const QString &password)
{
    Q_UNUSED(path)
    Q_UNUSED(desktopFile)
    Q_UNUSED(password)
    return false;
}

void VaultService::sessionRelockFile(const QString &path)
{
    Q_UNUSED(path)
}

void VaultService::relockAllSessions()
{
}

bool VaultService::isPathBlocked(const QString &path) const
{
    Q_UNUSED(path)
    return false;
}

bool VaultService::hasOwnPassword(const QString &path) const
{
    Q_UNUSED(path)
    return false;
}

QSet<QString> VaultService::allLockedPaths() const
{
    return {};
}

int VaultService::getRemainingLockoutSeconds(const QString &path) const
{
    Q_UNUSED(path)
    return 0;
}

QString VaultService::lastError() const
{
    return m_lastError;
}

void VaultService::clearLastError()
{
    m_lastError.clear();
}

void VaultService::purgePath(const QString &path)
{
    Q_UNUSED(path)
}

void VaultService::purgePaths(const QStringList &paths)
{
    Q_UNUSED(paths)
}
