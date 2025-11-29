/* pcmanfm/mainwindow_view.cpp */

#include "application.h"
#include "mainwindow.h"
#include "tabpage.h"

namespace PCManFM {

void MainWindow::on_actionIconView_triggered() {
    TabPage* page = currentPage();
    page->setViewMode(Fm::FolderView::IconMode);
    setTabIcon(page);
}

void MainWindow::on_actionCompactView_triggered() {
    TabPage* page = currentPage();
    page->setViewMode(Fm::FolderView::CompactMode);
    setTabIcon(page);
}

void MainWindow::on_actionDetailedList_triggered() {
    TabPage* page = currentPage();
    page->setViewMode(Fm::FolderView::DetailedListMode);
    setTabIcon(page);
}

void MainWindow::on_actionThumbnailView_triggered() {
    TabPage* page = currentPage();
    page->setViewMode(Fm::FolderView::ThumbnailMode);
    setTabIcon(page);
}

void MainWindow::on_actionShowHidden_triggered(bool checked) {
    currentPage()->setShowHidden(checked);
    // visibility of hidden folders in directory tree is toggled by onTabPageSortFilterChanged()
}

void MainWindow::on_actionShowThumbnails_triggered(bool checked) {
    QWidgetList windows = qApp->topLevelWidgets();
    QWidgetList::iterator it;
    for (it = windows.begin(); it != windows.end(); ++it) {
        QWidget* window = *it;
        if (window->inherits("PCManFM::MainWindow")) {
            MainWindow* mainWindow = static_cast<MainWindow*>(window);
            mainWindow->ui.actionShowThumbnails->setChecked(checked);  // doesn't call this function
            for (int i = 0; i < mainWindow->ui.viewSplitter->count(); ++i) {
                if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(mainWindow->ui.viewSplitter->widget(i))) {
                    int n = viewFrame->getStackedWidget()->count();
                    for (int j = 0; j < n; ++j) {
                        if (TabPage* page = static_cast<TabPage*>(viewFrame->getStackedWidget()->widget(j))) {
                            page->setShowThumbnails(checked);
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::on_actionByFileName_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileName, currentPage()->sortOrder());
}

void MainWindow::on_actionByMTime_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileMTime, currentPage()->sortOrder());
}

void MainWindow::on_actionByCrTime_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileCrTime, currentPage()->sortOrder());
}

void MainWindow::on_actionByDTime_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileDTime, currentPage()->sortOrder());
}

void MainWindow::on_actionByOwner_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileOwner, currentPage()->sortOrder());
}

void MainWindow::on_actionByGroup_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileGroup, currentPage()->sortOrder());
}

void MainWindow::on_actionByFileSize_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileSize, currentPage()->sortOrder());
}

void MainWindow::on_actionByFileType_triggered(bool /*checked*/) {
    currentPage()->sort(Fm::FolderModel::ColumnFileType, currentPage()->sortOrder());
}

void MainWindow::on_actionAscending_triggered(bool /*checked*/) {
    currentPage()->sort(currentPage()->sortColumn(), Qt::AscendingOrder);
}

void MainWindow::on_actionDescending_triggered(bool /*checked*/) {
    currentPage()->sort(currentPage()->sortColumn(), Qt::DescendingOrder);
}

void MainWindow::on_actionCaseSensitive_triggered(bool checked) { currentPage()->setSortCaseSensitive(checked); }

void MainWindow::on_actionFolderFirst_triggered(bool checked) { currentPage()->setSortFolderFirst(checked); }

void MainWindow::on_actionHiddenLast_triggered(bool checked) { currentPage()->setSortHiddenLast(checked); }

void MainWindow::on_actionPreserveView_triggered(bool checked) {
    TabPage* page = currentPage();
    page->setCustomizedView(checked);
    if (checked) {
        ui.actionPreserveViewRecursive->setChecked(false);
    }
    ui.actionGoToCustomizedViewSource->setVisible(page->hasInheritedCustomizedView());
    setTabIcon(page);
}

void MainWindow::on_actionPreserveViewRecursive_triggered(bool checked) {
    TabPage* page = currentPage();
    page->setCustomizedView(checked, true);
    if (checked) {
        ui.actionPreserveView->setChecked(false);
    }
    ui.actionGoToCustomizedViewSource->setVisible(page->hasInheritedCustomizedView());
    setTabIcon(page);
}

void MainWindow::on_actionGoToCustomizedViewSource_triggered() {
    currentPage()->goToCustomizedViewSource();
    updateUIForCurrentPage();
}

void MainWindow::on_actionFilter_triggered(bool checked) {
    static_cast<Application*>(qApp)->settings().setShowFilter(checked);
    // show/hide filter-bars and disable/enable their transience for all tabs
    // (of all view frames) in all windows because this is a global setting
    QWidgetList windows = qApp->topLevelWidgets();
    QWidgetList::iterator it;
    for (it = windows.begin(); it != windows.end(); ++it) {
        QWidget* window = *it;
        if (window->inherits("PCManFM::MainWindow")) {
            MainWindow* mainWindow = static_cast<MainWindow*>(window);
            mainWindow->ui.actionFilter->setChecked(checked);  // doesn't call this function
            for (int i = 0; i < mainWindow->ui.viewSplitter->count(); ++i) {
                if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(mainWindow->ui.viewSplitter->widget(i))) {
                    int n = viewFrame->getStackedWidget()->count();
                    for (int j = 0; j < n; ++j) {
                        if (TabPage* page = static_cast<TabPage*>(viewFrame->getStackedWidget()->widget(j))) {
                            page->transientFilterBar(!checked);
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::on_actionUnfilter_triggered() {
    // clear filters for all tabs (of all view frames)
    for (int i = 0; i < ui.viewSplitter->count(); ++i) {
        if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(ui.viewSplitter->widget(i))) {
            int n = viewFrame->getStackedWidget()->count();
            for (int j = 0; j < n; ++j) {
                if (TabPage* page = static_cast<TabPage*>(viewFrame->getStackedWidget()->widget(j))) {
                    page->clearFilter();
                }
            }
        }
    }
}

void MainWindow::on_actionShowFilter_triggered() {
    if (TabPage* page = currentPage()) {
        page->showFilterBar();
    }
}

}  // namespace PCManFM