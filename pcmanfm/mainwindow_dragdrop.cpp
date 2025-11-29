/* pcmanfm/mainwindow_dragdrop.cpp */

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTimer>

#include "application.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "tabpage.h"

namespace PCManFM {

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (!event) {
        return;
    }

    const auto* mime = event->mimeData();
    if (!mime) {
        return;
    }

    if (mime->hasFormat(QStringLiteral("application/pcmanfm-qt-tab"))
        // ensure that the tab drag source is ours (and not a root window, for example)
        && event->source() != nullptr) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    if (!event) {
        return;
    }

    const auto* mime = event->mimeData();
    if (mime && mime->hasFormat(QStringLiteral("application/pcmanfm-qt-tab"))) {
        if (QObject* sourceObject = event->source()) {
            // announce that the tab drop is accepted by us (see TabBar::mouseMoveEvent)
            sourceObject->setProperty(TabBar::tabDropped, true);
            // the tab will be dropped (moved) after the DND is finished
            QTimer::singleShot(0, sourceObject, [this, sourceObject]() { dropTab(sourceObject); });
        }
    }

    event->acceptProposedAction();
}

void MainWindow::dropTab(QObject* source) {
    if (!activeViewFrame_) {
        return;
    }

    auto* w = qobject_cast<QWidget*>(source);
    auto* dragSource = w ? qobject_cast<MainWindow*>(w->window()) : nullptr;

    if (dragSource == this || !dragSource) {  // drop on itself or invalid source
        activeViewFrame_->getTabBar()->finishMouseMoveEvent();
        return;
    }

    // first close the tab in the drag window;
    // then add its page to a new tab in the drop window
    TabPage* dropPage = dragSource->currentPage();
    if (dropPage) {
        disconnect(static_cast<QObject*>(dropPage), nullptr, dragSource, nullptr);

        // release mouse before tab removal because otherwise, the source tabbar
        // might not be updated properly with tab reordering during a fast drag-and-drop
        dragSource->activeViewFrame_->getTabBar()->releaseMouse();

        dragSource->activeViewFrame_->getStackedWidget()->removeWidget(static_cast<QWidget*>(dropPage));
        const int index = addTabWithPage(dropPage, activeViewFrame_);
        activeViewFrame_->getTabBar()->setCurrentIndex(index);
    } else {
        activeViewFrame_->getTabBar()->finishMouseMoveEvent();  // impossible
    }
}

void MainWindow::detachTab() {
    if (!activeViewFrame_) {
        return;
    }

    auto* app = qobject_cast<Application*>(qApp);
    Settings* settings = app ? &app->settings() : nullptr;

    // don't detach a single tab; split view state may have changed elsewhere
    if (activeViewFrame_->getStackedWidget()->count() == 1 || (settings && settings->splitView())) {
        activeViewFrame_->getTabBar()->finishMouseMoveEvent();
        return;
    }

    // close the tab and move its page to a new window
    TabPage* dropPage = currentPage();
    if (dropPage) {
        disconnect(static_cast<QObject*>(dropPage), nullptr, this, nullptr);

        activeViewFrame_->getTabBar()->releaseMouse();  // as in dropTab()
        activeViewFrame_->getStackedWidget()->removeWidget(static_cast<QWidget*>(dropPage));

        auto* newWin = new MainWindow();
        newWin->addTabWithPage(dropPage, newWin->activeViewFrame_);
        newWin->show();
    } else {
        activeViewFrame_->getTabBar()->finishMouseMoveEvent();  // impossible
    }
}

}  // namespace PCManFM
