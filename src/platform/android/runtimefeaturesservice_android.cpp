// ── Android stub ─────────────────────────────────────────────────────
// RuntimeFeaturesService on Android. The real implementation probes DBus
// (system bus interface for UDisks2) and checks for external binaries
// (ffmpeg, bat, udisksctl, wl-copy, git) to publish Q_PROPERTY feature
// flags consumed across QML. On Android none of these desktop tools or
// DBus services exist, so every flag is false. This keeps the exact QML
// interface (header unchanged): video/text-highlight previews and the
// integrated-window-controls path are all correctly reported as
// unavailable, and installHint() returns an empty string.

#include "services/runtimefeaturesservice.h"

RuntimeFeaturesService::RuntimeFeaturesService(QObject *parent)
    : QObject(parent)
{
}

bool RuntimeFeaturesService::ffmpegAvailable() const
{
    return false;
}

bool RuntimeFeaturesService::batAvailable() const
{
    return false;
}

bool RuntimeFeaturesService::udisksctlAvailable() const
{
    return false;
}

bool RuntimeFeaturesService::wlClipboardAvailable() const
{
    return false;
}

bool RuntimeFeaturesService::gitAvailable() const
{
    return false;
}

bool RuntimeFeaturesService::useIntegratedWindowControls() const
{
    // Android provides its own system title bar / edge-to-edge chrome;
    // the app does not draw integrated window controls.
    return false;
}

QString RuntimeFeaturesService::installHint(const QString &feature) const
{
    Q_UNUSED(feature)
    return {};
}
