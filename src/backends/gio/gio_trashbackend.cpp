/*
 * GIO-based trash backend implementation
 * src/backends/gio/gio_trashbackend.cpp
 */

#include "gio_trashbackend.h"

#include <gio/gio.h>

#include <QDebug>

namespace PCManFM {

bool GioTrashBackend::moveToTrash(const QString& path, QString* errorOut) {
    GFile* file = g_file_new_for_path(path.toUtf8().constData());
    if (!file) {
        if (errorOut) {
            *errorOut = QStringLiteral("Failed to create GFile for path: %1").arg(path);
        }
        return false;
    }

    GError* error = nullptr;
    gboolean success = g_file_trash(file, nullptr, &error);

    if (error) {
        if (errorOut) {
            *errorOut = QString::fromUtf8(error->message);
        }
        g_error_free(error);
    }

    g_object_unref(file);
    return success;
}

bool GioTrashBackend::restore(const QString& trashId, QString* errorOut) {
    Q_UNUSED(trashId);
    // TODO: Implement trash restoration
    // This requires parsing trash metadata and using g_file_move()
    // For now, return false as this is a more complex operation
    if (errorOut) {
        *errorOut = QStringLiteral("Trash restoration not yet implemented");
    }
    return false;
}

}  // namespace PCManFM