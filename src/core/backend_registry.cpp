/*
 * Backend registry implementation
 * src/core/backend_registry.cpp
 */

#include "backend_registry.h"

#include "../backends/gio/gio_trashbackend.h"
#include "../backends/qt/qt_fileops.h"
#include "../backends/qt/qt_foldermodel.h"

namespace PCManFM {

namespace {
GioTrashBackend* g_trashBackend = nullptr;
}

void BackendRegistry::initDefaults() {
    // Currently just ensures the backend system is available
    // In the future, this could initialize different backends based on configuration
    if (!g_trashBackend) {
        g_trashBackend = new GioTrashBackend();
    }
    qDebug() << "BackendRegistry initialized";
}

std::unique_ptr<IFileOps> BackendRegistry::createFileOps() { return std::make_unique<QtFileOps>(); }

std::unique_ptr<IFolderModel> BackendRegistry::createFolderModel(QObject* parent) {
    return std::make_unique<QtFolderModel>(parent);
}

ITrashBackend* BackendRegistry::trash() { return g_trashBackend; }

IVolumeBackend* BackendRegistry::volume() {
    // TODO: Implement volume backend
    return nullptr;
}

IRemoteBackend* BackendRegistry::remote() {
    // TODO: Implement remote backend
    return nullptr;
}

}  // namespace PCManFM