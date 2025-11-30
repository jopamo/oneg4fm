/*
 * Volume backend interface for PCManFM-Qt backend architecture
 * pcmanfm-qt/src/core/ivolumebackend.h
 */

#pragma once

#include <QList>
#include <QString>

struct VolumeInfo {
    QString id;
    QString name;
    QString device;
    QString mountPoint;
    bool mounted;
    bool removable;
};

class IVolumeBackend {
   public:
    virtual ~IVolumeBackend() = default;

    virtual QList<VolumeInfo> listVolumes() = 0;
    virtual bool mount(const QString& id, QString* errorOut) = 0;
    virtual bool unmount(const QString& id, QString* errorOut) = 0;
    virtual bool eject(const QString& id, QString* errorOut) = 0;
};