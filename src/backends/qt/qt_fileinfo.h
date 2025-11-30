/*
 * Qt-based file info implementation for PCManFM-Qt
 * pcmanfm-qt/src/backends/qt/qt_fileinfo.h
 */

#pragma once

#include <QFileInfo>
#include <QMimeDatabase>

#include "../../core/ifileinfo.h"

class QtFileInfo : public IFileInfo {
   public:
    explicit QtFileInfo(const QString& path);
    explicit QtFileInfo(const QFileInfo& fileInfo);

    QString path() const override;
    QString name() const override;
    QString displayName() const override;

    bool isDir() const override;
    bool isFile() const override;
    bool isSymlink() const override;
    bool isHidden() const override;

    qint64 size() const override;
    QDateTime lastModified() const override;

    QString mimeType() const override;
    QIcon icon() const override;

   private:
    QFileInfo m_fileInfo;
    mutable QMimeDatabase m_mimeDatabase;
};