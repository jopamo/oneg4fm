/* pcmanfm/mainwindow_events.cpp */

#include <QCloseEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTextEdit>

#include "application.h"
#include "mainwindow.h"

namespace PCManFM {

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    auto* watchedWidget = qobject_cast<QWidget*>(watched);
    if (watchedWidget) {
        if (event->type() == QEvent::FocusIn
            // the event has happened inside the splitter
            && ui.viewSplitter->isAncestorOf(watchedWidget)) {
            for (int i = 0; i < ui.viewSplitter->count(); ++i) {
                if (auto* viewFrame = qobject_cast<ViewFrame*>(ui.viewSplitter->widget(i))) {
                    if (viewFrame->isAncestorOf(watchedWidget)) {
                        // a widget inside this view frame has gained focus; ensure the view is active
                        if (activeViewFrame_ != viewFrame) {
                            activeViewFrame_ = viewFrame;
                            updateUIForCurrentPage(false);  // WARNING: never set focus here!
                        }
                        if (viewFrame->palette().color(QPalette::Base) != qApp->palette().color(QPalette::Base)) {
                            viewFrame->setPalette(qApp->palette());  // restore the main palette
                        }
                    } else if (viewFrame->palette().color(QPalette::Base) == qApp->palette().color(QPalette::Base)) {
                        // Change the text and base palettes of an inactive view frame a little
                        // NOTE: Style-sheets aren't used because they can interfere with QStyle
                        QPalette palette = viewFrame->palette();

                        // There are various ways of getting a distinct color near the base color
                        // but this one gives the best results with almost all palettes
                        QColor txtCol = palette.color(QPalette::Text);
                        QColor baseCol = palette.color(QPalette::Base);
                        baseCol.setRgbF(0.9 * baseCol.redF() + 0.1 * txtCol.redF(),
                                        0.9 * baseCol.greenF() + 0.1 * txtCol.greenF(),
                                        0.9 * baseCol.blueF() + 0.1 * txtCol.blueF(), baseCol.alphaF());
                        palette.setColor(QPalette::Base, baseCol);

                        // view text
                        txtCol.setAlphaF(txtCol.alphaF() * 0.7);
                        palette.setColor(QPalette::Text, txtCol);

                        // window text (used in tabs)
                        txtCol = palette.color(QPalette::WindowText);
                        txtCol.setAlphaF(txtCol.alphaF() * 0.7);
                        palette.setColor(QPalette::WindowText, txtCol);

                        // button text (the disabled text color isn't changed because it may be
                        // used by some styles for drawing disabled path-bar arrow)
                        txtCol = palette.color(QPalette::ButtonText);
                        txtCol.setAlphaF(txtCol.alphaF() * 0.7);
                        palette.setColor(QPalette::Active, QPalette::ButtonText, txtCol);
                        palette.setColor(QPalette::Inactive, QPalette::ButtonText, txtCol);

                        viewFrame->setPalette(palette);
                    }
                }
            }
        }
        // Use the Tab key for switching between view frames
        else if (event->type() == QEvent::KeyPress) {
            auto* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Tab && ke->modifiers() == Qt::NoModifier) {
                if (!qobject_cast<QTextEdit*>(watchedWidget)  // not during inline renaming
                    && ui.viewSplitter->isAncestorOf(watchedWidget)) {
                    // wrap the focus
                    for (int i = 0; i < ui.viewSplitter->count(); ++i) {
                        if (auto* viewFrame = qobject_cast<ViewFrame*>(ui.viewSplitter->widget(i))) {
                            if (activeViewFrame_ == viewFrame) {
                                const int n = i < ui.viewSplitter->count() - 1 ? i + 1 : 0;
                                activeViewFrame_ = qobject_cast<ViewFrame*>(ui.viewSplitter->widget(n));
                                updateUIForCurrentPage();  // focuses the view and calls this function again
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    Settings& settings = app->settings();
    if (settings.rememberWindowSize()) {
        settings.setLastWindowMaximized(isMaximized());

        if (!isMaximized()) {
            settings.setLastWindowWidth(width());
            settings.setLastWindowHeight(height());
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (lastActive_ == this) {
        lastActive_ = nullptr;
    }

    QMainWindow::closeEvent(event);

    auto* app = qobject_cast<Application*>(qApp);
    if (!app) {
        return;
    }

    Settings& settings = app->settings();
    if (settings.rememberWindowSize()) {
        settings.setLastWindowMaximized(isMaximized());

        if (!isMaximized()) {
            settings.setLastWindowWidth(width());
            settings.setLastWindowHeight(height());
        }
    }

    // remember last tab paths only if this is the last window
    QStringList tabPaths;
    int splitNum = 0;
    if (lastActive_ == nullptr && settings.reopenLastTabs()) {
        for (int i = 0; i < ui.viewSplitter->count(); ++i) {
            if (auto* viewFrame = qobject_cast<ViewFrame*>(ui.viewSplitter->widget(i))) {
                const int n = viewFrame->getStackedWidget()->count();
                for (int j = 0; j < n; ++j) {
                    if (auto* page = qobject_cast<TabPage*>(viewFrame->getStackedWidget()->widget(j))) {
                        tabPaths.append(QString::fromUtf8(page->path().toString().get()));
                    }
                }
            }
            if (i == 0 && ui.viewSplitter->count() > 1) {
                splitNum = tabPaths.size();
            }
        }
    }
    settings.setTabPaths(tabPaths);
    settings.setSplitViewTabsNum(splitNum);
}

bool MainWindow::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::WindowActivate:
            lastActive_ = this;
            break;
        default:
            break;
    }
    return QMainWindow::event(event);
}

void MainWindow::changeEvent(QEvent* event) {
    switch (event->type()) {
        case QEvent::LayoutDirectionChange:
            setRTLIcons(QApplication::layoutDirection() == Qt::RightToLeft);
            break;
        default:
            break;
    }
    QWidget::changeEvent(event);
}

}  // namespace PCManFM
