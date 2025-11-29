#include "launcher.h"

#include <libfm-qt6/core/filepath.h>

#include <QFileInfo>

#include "application.h"
#include "mainwindow.h"

namespace PCManFM {

Launcher::Launcher(PCManFM::MainWindow* mainWindow)
    : Fm::FileLauncher(), mainWindow_(mainWindow), openInNewTab_(false), openWithDefaultFileManager_(false) {
    auto* app = static_cast<Application*>(qApp);
    setQuickExec(app->settings().quickExec());
}

Launcher::~Launcher() = default;

// open a list of folders either in an existing main window, in new tabs, or with the system file manager
bool Launcher::openFolder(GAppLaunchContext* ctx, const Fm::FileInfoList& folderInfos, Fm::GErrorPtr& /*err*/) {
    if (folderInfos.empty()) {
        return false;
    }

    auto* app = static_cast<Application*>(qApp);
    MainWindow* mainWindow = mainWindow_;

    auto firstInfo = folderInfos.front();
    Fm::FilePath path = firstInfo->path();

    if (!mainWindow) {
        // If there is no PCManFM-Qt main window, we may delegate to the system default file manager
        // This is done when:
        //   1) openWithDefaultFileManager_ is set
        //   2) we are not explicitly opening in new tabs
        //   3) the default file manager exists and is not pcmanfm-qt itself
        if (openWithDefaultFileManager_ && !openInNewTab_) {
            auto defaultApp = Fm::GAppInfoPtr{g_app_info_get_default_for_type("inode/directory", FALSE), false};
            if (defaultApp != nullptr && strcmp(g_app_info_get_id(defaultApp.get()), "pcmanfm-qt.desktop") != 0) {
                for (const auto& folder : folderInfos) {
                    Fm::FileLauncher::launchWithDefaultApp(folder, ctx);
                }
                return true;
            }
        }

        // fall back to opening a new PCManFM-Qt main window
        mainWindow = new MainWindow(std::move(path));
        mainWindow->resize(app->settings().windowWidth(), app->settings().windowHeight());

        if (app->settings().windowMaximized()) {
            mainWindow->setWindowState(mainWindow->windowState() | Qt::WindowMaximized);
        }
    } else {
        // we already have a main window, either reuse the current tab or open a new one
        if (openInNewTab_) {
            mainWindow->addTab(std::move(path));
        } else {
            mainWindow->chdir(std::move(path));
        }
    }

    // remaining folders always open in new tabs in the same window
    for (size_t i = 1; i < folderInfos.size(); ++i) {
        auto& fi = folderInfos[i];
        Fm::FilePath extraPath = fi->path();
        mainWindow->addTab(std::move(extraPath));
    }

    mainWindow->show();
    mainWindow->raise();
    mainWindow->activateWindow();

    // reset the tab flag for subsequent launches
    openInNewTab_ = false;

    return true;
}

void Launcher::launchedFiles(const Fm::FileInfoList& files) const {
    auto* app = static_cast<Application*>(qApp);
    if (app->settings().getRecentFilesNumber() <= 0) {
        return;
    }

    for (const auto& file : files) {
        if (file->isNative() && !file->isDir()) {
            app->settings().addRecentFile(QString::fromUtf8(file->path().localPath().get()));
        }
    }
}

void Launcher::launchedPaths(const Fm::FilePathList& paths) const {
    auto* app = static_cast<Application*>(qApp);
    if (app->settings().getRecentFilesNumber() <= 0) {
        return;
    }

    for (const auto& path : paths) {
        if (!path.isNative()) {
            continue;
        }

        const QString pathStr = QString::fromUtf8(path.localPath().get());
        // QFileInfo check is cheap here because the path is native
        if (!QFileInfo(pathStr).isDir()) {
            app->settings().addRecentFile(pathStr);
        }
    }
}

}  // namespace PCManFM
