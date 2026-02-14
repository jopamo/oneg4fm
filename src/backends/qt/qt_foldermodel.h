/*
 * Qt-based folder model implementation for oneg4fm
 * oneg4fm/src/backends/qt/qt_foldermodel.h
 */

#pragma once

#include <QFileSystemModel>

#include "../../core/ifoldermodel.h"

class QtFolderModel : public IFolderModel {
    Q_OBJECT

   public:
    explicit QtFolderModel(QObject* parent = nullptr);
    ~QtFolderModel() override;

    void setDirectory(const QString& path) override;
    QString directory() const override;

    void refresh() override;

    // QAbstractItemModel implementation
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

   private:
    QFileSystemModel* m_fileSystemModel;
};