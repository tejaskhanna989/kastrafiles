// ── Android stub ─────────────────────────────────────────────────────
// DependencyChecker on Android. The real implementation shells out to
// `which` for a long list of Linux CLI tools (ffmpeg, bat, ntfs-3g, ...)
// and queries DBus for the UDisks2 service, then feeds the
// MissingDependenciesDialog with per-distro install commands. None of that
// applies on Android, where apps cannot install system packages. This stub
// keeps the exact QML interface (header unchanged) but reports an empty
// dependency list, so hasAnyMissing()/hasMissingRequired() are false and
// the startup dependency check stays quiet.

#include "services/dependencychecker.h"

DependencyChecker::DependencyChecker(QObject *parent)
    : QObject(parent)
{
    // Android has no distro and no installable CLI toolchain; leave the
    // dependency list empty so nothing is reported as missing.
    m_distroId = QStringLiteral("android");
    m_distroName = QStringLiteral("Android");
}

QVariantList DependencyChecker::dependencies() const
{
    return {};
}

QVariantList DependencyChecker::missingDependencies() const
{
    return {};
}

bool DependencyChecker::hasAnyMissing() const
{
    return false;
}

bool DependencyChecker::hasMissingRequired() const
{
    return false;
}

void DependencyChecker::refresh()
{
    // Nothing to probe on Android.
}

QString DependencyChecker::installCommandFor(const QString &id) const
{
    Q_UNUSED(id)
    return {};
}
