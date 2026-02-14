/*
 * Remote backend interface for oneg4fm backend architecture
 * oneg4fm/src/core/iremotebackend.h
 */

#pragma once

#include <QString>
#include <QUrl>

class IRemoteBackend {
   public:
    virtual ~IRemoteBackend() = default;

    virtual bool isRemote(const QUrl& url) const = 0;
    virtual QString mapToMountPoint(const QUrl& url) = 0;
};