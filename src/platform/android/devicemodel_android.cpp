// ── Android stub ─────────────────────────────────────────────────────
// DeviceModel on Android. The real implementation drives GIO's
// GVolumeMonitor and UDisks2 over DBus to enumerate mountable drives —
// neither API exists on Android. This stub keeps the exact same interface
// (header unchanged) so Sidebar.qml / Main.qml bindings keep working, but
// reports zero devices. On Android the app browses shared storage directly,
// so there is no concept of a removable volume to list or mount.

#include "models/devicemodel.h"

DeviceModel::DeviceModel(QObject *parent, bool deferInitialRefresh)
    : QAbstractListModel(parent)
{
    Q_UNUSED(deferInitialRefresh)
}

DeviceModel::~DeviceModel() = default;

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.count())
        return QVariant();

    const DeviceEntry &device = m_devices.at(index.row());
    switch (role) {
    case DeviceNameRole:    return device.deviceName;
    case DevicePathRole:    return device.devicePath;
    case MountPointRole:    return device.mountPoint;
    case TotalSizeRole:     return device.totalSize;
    case FreeSpaceRole:     return device.freeSpace;
    case UsagePercentRole:  return device.usagePercent;
    case RemovableRole:     return device.removable;
    case MountedRole:       return device.mounted;
    case BackendRole:       return static_cast<int>(device.backend);
    default:                return QVariant();
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {
        { DeviceNameRole,    "deviceName" },
        { DevicePathRole,    "devicePath" },
        { MountPointRole,    "mountPoint" },
        { TotalSizeRole,     "totalSize" },
        { FreeSpaceRole,     "freeSpace" },
        { UsagePercentRole,  "usagePercent" },
        { RemovableRole,     "removable" },
        { MountedRole,       "mounted" },
        { BackendRole,       "backend" },
    };
}

void DeviceModel::unmount(int index)
{
    Q_UNUSED(index)
    // No mounts exist on Android, so unmount is a no-op.
}

void DeviceModel::mount(int index)
{
    Q_UNUSED(index)
    // No devices to mount on Android.
}

void DeviceModel::refresh()
{
    // Intentionally empty: m_devices stays empty on Android.
}

void DeviceModel::scheduleRefresh()
{
    // No-op.
}

void DeviceModel::clearDevices()
{
    // Kept for interface parity; never populated on Android.
}
