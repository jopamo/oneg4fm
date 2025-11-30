/*
 * GIO-based trash backend
 * src/backends/gio/gio_trashbackend.h
 */

#ifndef GIO_TRASHBACKEND_H
#define GIO_TRASHBACKEND_H

#include "../../core/itrashbackend.h"

namespace PCManFM {

class GioTrashBackend : public ITrashBackend {
   public:
    GioTrashBackend() = default;
    ~GioTrashBackend() override = default;

    bool moveToTrash(const QString& path, QString* errorOut) override;
    bool restore(const QString& trashId, QString* errorOut) override;
};

}  // namespace PCManFM

#endif  // GIO_TRASHBACKEND_H