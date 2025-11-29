/* pcmanfm/mainwindow_bookmarks.cpp */

#include <libfm-qt6/bookmarks.h>

#include "application.h"
#include "mainwindow.h"

namespace PCManFM {

void MainWindow::loadBookmarksMenu() {
    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    auto& bookmarks = app->bookmarks();
    const auto bookmarkActions = ui.menuBookmarks->actions();
    const auto bookmarksList = bookmarks.allBookmarks();
    const int bookmarksSize = bookmarksList.size();

    // remove old actions that belong to bookmarks
    for (QAction* action : bookmarkActions) {
        if (action->data().isValid()) {
            ui.menuBookmarks->removeAction(action);
        }
    }

    // add new actions
    for (int i = 0; i < bookmarksSize; ++i) {
        const auto& bookmark = bookmarksList.at(i);
        auto* action = new QAction(bookmark.name(), this);
        action->setData(bookmark.path());
        connect(action, &QAction::triggered, this, &MainWindow::onBookmarkActionTriggered);
        ui.menuBookmarks->insertAction(ui.actionAddToBookmarks, action);
    }
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
    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto path = page->path();
    if (!path) {
        return;
    }

    auto& bookmarks = app->bookmarks();
    const QString name = page->pathName();
    bookmarks.addBookmark(name, path);
}

}  // namespace PCManFM
