/*
 * GIO remote backend implementation
 * src/backends/gio/gio_remotebackend.h
 */

#pragma once

#include <gio/gio.h>

#include "../../core/iremotebackend.h"

namespace PCManFM {

class GioRemoteBackend : public IRemoteBackend {
   public:
    GioRemoteBackend();
    ~GioRemoteBackend() override;

    bool isRemote(const QUrl& url) const override;
    QString mapToMountPoint(const QUrl& url) override;

   private:
    GVolumeMonitor* volumeMonitor_ = nullptr;
};

}  // namespace PCManFM