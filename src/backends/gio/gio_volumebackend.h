/*
 * GIO volume backend implementation
 * src/backends/gio/gio_volumebackend.h
 */

#pragma once

#include <gio/gio.h>

#include "../../core/ivolumebackend.h"

namespace PCManFM {

class GioVolumeBackend : public IVolumeBackend {
   public:
    GioVolumeBackend();
    ~GioVolumeBackend() override;

    QList<VolumeInfo> listVolumes() override;
    bool mount(const QString& id, QString* errorOut) override;
    bool unmount(const QString& id, QString* errorOut) override;
    bool eject(const QString& id, QString* errorOut) override;

   private:
    GVolumeMonitor* volumeMonitor_ = nullptr;
};

}  // namespace PCManFM