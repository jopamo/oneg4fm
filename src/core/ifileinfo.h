/*
 * File info interface for PCManFM-Qt backend architecture
 * pcmanfm-qt/src/core/ifileinfo.h
 */

#pragma once

#include <QDateTime>
#include <QIcon>
#include <QString>

class IFileInfo {
   public:
    virtual ~IFileInfo() = default;

    virtual QString path() const = 0;
    virtual QString name() const = 0;
    virtual QString displayName() const = 0;

    virtual bool isDir() const = 0;
    virtual bool isFile() const = 0;
    virtual bool isSymlink() const = 0;
    virtual bool isHidden() const = 0;

    virtual qint64 size() const = 0;
    virtual QDateTime lastModified() const = 0;

    virtual QString mimeType() const = 0;
    virtual QIcon icon() const = 0;
};