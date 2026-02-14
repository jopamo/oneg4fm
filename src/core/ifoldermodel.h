/*
 * Folder model interface for oneg4fm backend architecture
 * oneg4fm/src/core/ifoldermodel.h
 */

#pragma once

#include <QAbstractItemModel>

class IFolderModel : public QAbstractItemModel {
    Q_OBJECT
   public:
    explicit IFolderModel(QObject* parent = nullptr) : QAbstractItemModel(parent) {}
    ~IFolderModel() override = default;

    virtual void setDirectory(const QString& path) = 0;
    virtual QString directory() const = 0;

    virtual void refresh() = 0;
};