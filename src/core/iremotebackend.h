/*
 * Remote backend interface for PCManFM-Qt backend architecture
 * pcmanfm-qt/src/core/iremotebackend.h
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