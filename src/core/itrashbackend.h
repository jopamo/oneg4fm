/*
 * Trash backend interface
 * src/core/itrashbackend.h
 */

#ifndef ITRASHBACKEND_H
#define ITRASHBACKEND_H

#include <QString>

namespace PCManFM {

class ITrashBackend {
   public:
    virtual ~ITrashBackend() = default;

    virtual bool moveToTrash(const QString& path, QString* errorOut) = 0;
    virtual bool restore(const QString& trashId, QString* errorOut) = 0;
};

}  // namespace PCManFM

#endif  // ITRASHBACKEND_H