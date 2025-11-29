/* pcmanfm/mainwindow_bookmarks.cpp */

#include <QAction>
#include <QMenu>

#include "application.h"
#include "mainwindow.h"
#include "tabpage.h"

namespace PCManFM {

void MainWindow::loadBookmarksMenu() {
    // Clear previously inserted dynamic bookmark actions
    auto* menu = ui.menu_Bookmarks;
    const auto actions = menu->actions();

    for (QAction* action : actions) {
        // use a custom property to identify bookmark actions if you want
        if (action->property("pcmanfm_bookmark").toBool()) {
            menu->removeAction(action);
            delete action;
        }
    }

    // TODO: if you want real bookmark entries here, repopulate from your
    //       backend (libfm-qt bookmarks, config, etc.)
}

void MainWindow::onBookmarksChanged() { loadBookmarksMenu(); }

void MainWindow::onBookmarkActionTriggered() {
    auto* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    const QString pathStr = action->data().toString();
    if (pathStr.isEmpty()) {
        return;
    }

    const QByteArray pathArray = pathStr.toLocal8Bit();
    chdir(Fm::FilePath::fromPathStr(pathArray.constData()));
}

void MainWindow::on_actionAddToBookmarks_triggered() {
    auto* app = static_cast<Application*>(qApp);

    // Delegate to Application's existing bookmark editor
    app->editBookmarks();
}

}  // namespace PCManFM
