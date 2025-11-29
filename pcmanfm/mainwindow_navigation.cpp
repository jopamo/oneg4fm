/* pcmanfm/mainwindow_navigation.cpp */

#include <QStandardPaths>
#include <QTimer>

#include "application.h"
#include "mainwindow.h"
#include "tabpage.h"

namespace PCManFM {

void MainWindow::chdir(Fm::FilePath path, ViewFrame* viewFrame) {
    if (!viewFrame) {
        return;
    }

    // wait until queued events are processed
    QTimer::singleShot(0, viewFrame, [this, path, viewFrame] {
        if (TabPage* page = currentPage(viewFrame)) {
            page->chdir(path, true);
            setTabIcon(page);
            if (viewFrame == activeViewFrame_) {
                updateUIForCurrentPage();
            } else {
                if (auto* pathBar = qobject_cast<Fm::PathBar*>(viewFrame->getTopBar())) {
                    pathBar->setPath(page->path());
                } else if (auto* pathEntry = qobject_cast<Fm::PathEdit*>(viewFrame->getTopBar())) {
                    pathEntry->setText(page->pathName());
                }
            }
        }
    });
}

void MainWindow::on_actionGoUp_triggered() {
    QTimer::singleShot(0, this, [this] {
        if (TabPage* page = currentPage()) {
            page->up();
            setTabIcon(page);
            updateUIForCurrentPage();
        }
    });
}

void MainWindow::on_actionGoBack_triggered() {
    QTimer::singleShot(0, this, [this] {
        if (TabPage* page = currentPage()) {
            page->backward();
            setTabIcon(page);
            updateUIForCurrentPage();
        }
    });
}

void MainWindow::on_actionGoForward_triggered() {
    QTimer::singleShot(0, this, [this] {
        if (TabPage* page = currentPage()) {
            page->forward();
            setTabIcon(page);
            updateUIForCurrentPage();
        }
    });
}

void MainWindow::on_actionHome_triggered() { chdir(Fm::FilePath::homeDir()); }

void MainWindow::on_actionReload_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    page->reload();
    if (pathEntry_ != nullptr) {
        pathEntry_->setText(page->pathName());
    }
}

void MainWindow::on_actionConnectToServer_triggered() {
    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    app->connectToServer();
}

void MainWindow::on_actionComputer_triggered() { chdir(Fm::FilePath::fromUri("computer:///")); }

void MainWindow::on_actionApplications_triggered() { chdir(Fm::FilePath::fromUri("menu://applications/")); }

void MainWindow::on_actionTrash_triggered() { chdir(Fm::FilePath::fromUri("trash:///")); }

void MainWindow::on_actionNetwork_triggered() { chdir(Fm::FilePath::fromUri("network:///")); }

void MainWindow::on_actionDesktop_triggered() {
    const QByteArray desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).toLocal8Bit();
    if (!desktop.isEmpty()) {
        chdir(Fm::FilePath::fromLocalPath(desktop.constData()));
    }
}

void MainWindow::on_actionOpenAsAdmin_triggered() {
    if (TabPage* page = currentPage()) {
        if (auto path = page->path()) {
            if (path.isNative()) {
                CStrPtr admin{g_strconcat("admin://", path.localPath().get(), nullptr)};
                chdir(Fm::FilePath::fromPathStr(admin.get()));
            }
        }
    }
}

void MainWindow::on_actionOpenAsRoot_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    Settings& settings = app->settings();
    if (!settings.suCommand().isEmpty()) {
        // run the su command
        // FIXME: it's better to get the filename of the current process rather than hard-code pcmanfm-qt here
        QByteArray suCommand = settings.suCommand().toLocal8Bit();
        QByteArray programCommand = app->applicationFilePath().toLocal8Bit();
        programCommand += " %U";

        // if %s exists in the su command, substitute it with the program
        const int substPos = suCommand.indexOf("%s");
        if (substPos != -1) {
            // replace %s with program
            suCommand.replace(substPos, 2, programCommand);
        } else {
            // no %s found so just append to it
            suCommand += programCommand;
        }

        Fm::GAppInfoPtr appInfo{
            g_app_info_create_from_commandline(suCommand.constData(), nullptr, GAppInfoCreateFlags(0), nullptr), false};

        if (appInfo) {
            auto cwd = page->path();
            Fm::GErrorPtr err;
            auto uri = cwd.uri();
            GList* uris = g_list_prepend(nullptr, uri.get());

            if (!g_app_info_launch_uris(appInfo.get(), uris, nullptr, &err)) {
                QMessageBox::critical(this, tr("Error"), QString::fromUtf8(err->message));
            }

            g_list_free(uris);
        }
    } else {
        // show an error message and ask the user to set the command
        QMessageBox::critical(this, tr("Error"), tr("Switch user command is not set"));
        app->preferences(QStringLiteral("advanced"));
    }
}

void MainWindow::on_actionFindFiles_triggered() {
    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    const auto files = page->selectedFiles();
    QStringList paths;

    if (!files.empty()) {
        for (const auto& file : files) {
            // FIXME: is it ok to use display name here?
            // This might be broken on filesystems with non-UTF-8 filenames
            if (file->isDir()) {
                paths.append(QString::fromUtf8(file->path().displayName().get()));
            }
        }
    }

    if (paths.isEmpty()) {
        paths.append(page->pathName());
    }

    app->findFiles(paths);
}

void MainWindow::on_actionOpenTerminal_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    app->openFolderInTerminal(page->path());
}

}  // namespace PCManFM
