/*
 * Main window file operations implementation
 * pcmanfm/mainwindow_fileops.cpp
 */

#include "application.h"
#include "bulkrename.h"
#include "mainwindow.h"
#include "tabpage.h"

// New backend headers
#include "../src/backends/qt/qt_fileinfo.h"
#include "../src/core/backend_registry.h"
#include "../src/ui/filepropertiesdialog.h"

// LibFM-Qt Headers
#include <libfm-qt6/fileoperation.h>
#include <libfm-qt6/filepropsdialog.h>
#include <libfm-qt6/utilities.h>

// Qt Headers
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>

namespace PCManFM {

namespace {

// Helper to access Application settings concisely
Settings& appSettings() { return static_cast<Application*>(qApp)->settings(); }

// Helper to convert Fm::FilePathList to QStringList
QStringList filePathListToStringList(const Fm::FilePathList& paths) {
    QStringList result;
    for (const auto& path : paths) {
        result.append(QString::fromUtf8(path.toString().get()));
    }
    return result;
}

// Temporary adapter to convert libfm file info to new backend IFileInfo
std::shared_ptr<IFileInfo> convertToIFileInfo(const std::shared_ptr<const Fm::FileInfo>& fmInfo) {
    // For now, create a simple QtFileInfo from the path
    // TODO: Properly convert all metadata when QtFileInfo is fully integrated
    return std::make_shared<QtFileInfo>(QString::fromUtf8(fmInfo->path().toString().get()));
}

// Rename file using new backend
bool renameFileWithBackend(const std::shared_ptr<const Fm::FileInfo>& file, QWidget* parent) {
    // For now, use a simple QInputDialog for renaming
    // TODO: Implement proper rename dialog with validation
    bool ok;

    // Get current name from file path
    QString currentPath = QString::fromUtf8(file->path().toString().get());
    QFileInfo fileInfo(currentPath);
    QString currentName = fileInfo.fileName();

    QString newName =
        QInputDialog::getText(parent, QApplication::translate("MainWindow", "Rename"),
                              QApplication::translate("MainWindow", "New name:"), QLineEdit::Normal, currentName, &ok);

    if (ok && !newName.isEmpty() && newName != currentName) {
        QString newPath = fileInfo.absolutePath() + QLatin1String("/") + newName;

        // Use QFile for the rename operation
        QFile fileObj(currentPath);
        if (fileObj.rename(newPath)) {
            return true;
        } else {
            QMessageBox::critical(
                parent, QApplication::translate("MainWindow", "Error"),
                QApplication::translate("MainWindow", "Failed to rename file: %1").arg(fileObj.errorString()));
            return false;
        }
    }

    return ok;
}

}  // namespace

void MainWindow::on_actionFileProperties_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto files = page->selectedFiles();
    if (!files.empty()) {
        // Convert libfm file info to new backend IFileInfo
        QList<std::shared_ptr<IFileInfo>> fileInfos;
        for (const auto& file : files) {
            fileInfos.append(convertToIFileInfo(file));
        }

        auto dialog = new FilePropertiesDialog(fileInfos, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}

void MainWindow::on_actionFolderProperties_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto folder = page->folder();
    if (!folder) {
        return;
    }

    auto info = folder->info();
    if (info) {
        auto fileInfo = convertToIFileInfo(info);
        auto dialog = new FilePropertiesDialog(fileInfo, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}

void MainWindow::on_actionCopy_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto paths = page->selectedFilePaths();
    if (!paths.empty()) {
        copyFilesToClipboard(paths);
    }
}

void MainWindow::on_actionCut_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto paths = page->selectedFilePaths();
    if (!paths.empty()) {
        cutFilesToClipboard(paths);
    }
}

void MainWindow::on_actionPaste_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    pasteFilesFromClipboard(page->path());
}

void MainWindow::on_actionDelete_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    Settings& settings = appSettings();
    auto paths = page->selectedFilePaths();
    if (paths.empty()) {
        return;
    }

    // Check if files are already in trash
    const bool trashed =
        std::any_of(paths.cbegin(), paths.cend(), [](const auto& path) { return path.hasUriScheme("trash"); });

    const bool shiftPressed = (qApp->keyboardModifiers() & Qt::ShiftModifier);

    if (settings.useTrash() && !shiftPressed && !trashed) {
        // Use new trash backend
        auto trashBackend = BackendRegistry::trash();
        QStringList pathStrings = filePathListToStringList(paths);

        for (const QString& path : pathStrings) {
            QString error;
            if (!trashBackend->moveToTrash(path, &error)) {
                QMessageBox::warning(this, tr("Move to Trash Failed"),
                                     tr("Failed to move '%1' to trash: %2").arg(path, error));
                return;
            }
        }
    } else {
        // Use new file operations backend for permanent deletion
        auto fileOps = BackendRegistry::createFileOps();
        FileOpRequest req;
        req.type = FileOpType::Delete;
        req.sources = filePathListToStringList(paths);
        req.destination = QString();  // Not used for delete
        req.followSymlinks = false;
        req.overwriteExisting = false;

        // TODO: Add progress dialog and error handling
        fileOps->start(req);
    }
}

void MainWindow::on_actionRename_triggered() {
    // do inline renaming if only one item is selected
    // otherwise use the renaming dialog
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto files = page->selectedFiles();

    // Case 1: Inline rename
    if (files.size() == 1) {
        QAbstractItemView* view = page->folderView()->childView();
        if (!view || !view->selectionModel()) {
            return;
        }

        QModelIndexList selIndexes = view->selectionModel()->selectedIndexes();

        // In the detailed list mode, multiple columns might be selected for one row.
        // We ensure we edit the primary column (filename).
        if (!selIndexes.isEmpty()) {
            const QModelIndex editIndex = selIndexes.first();
            view->setCurrentIndex(editIndex);
            view->scrollTo(editIndex);
            view->edit(editIndex);
        }
        return;
    }

    // Case 2: Multi-file rename (sequential dialogs)
    // NOTE: For true bulk rename, use on_actionBulkRename_triggered
    if (!files.empty()) {
        for (auto& file : files) {
            if (!renameFileWithBackend(file, this)) {
                break;
            }
        }
    }
}

void MainWindow::on_actionBulkRename_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto files = page->selectedFiles();
    if (!files.empty()) {
        BulkRenamer(files, this);
    }
}

void MainWindow::on_actionSelectAll_triggered() {
    if (TabPage* page = currentPage()) {
        page->selectAll();
    }
}

void MainWindow::on_actionDeselectAll_triggered() {
    if (TabPage* page = currentPage()) {
        page->deselectAll();
    }
}

void MainWindow::on_actionInvertSelection_triggered() {
    if (TabPage* page = currentPage()) {
        page->invertSelection();
    }
}

void MainWindow::on_actionCopyFullPath_triggered() {
    TabPage* page = currentPage();
    if (!page) {
        return;
    }

    auto paths = page->selectedFilePaths();
    if (paths.size() == 1) {
        // Use QString fromUtf8 explicitly
        QApplication::clipboard()->setText(QString::fromUtf8(paths.front().toString().get()));
    }
}

void MainWindow::on_actionCleanPerFolderConfig_triggered() {
    const auto r = QMessageBox::question(this, tr("Cleaning Folder Settings"),
                                         tr("Do you want to remove settings of nonexistent folders?\nThey might be "
                                            "useful if those folders are created again."),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (r == QMessageBox::Yes) {
        static_cast<Application*>(qApp)->cleanPerFolderConfig();
    }
}

void MainWindow::openFolderAndSelectFiles(const Fm::FilePathList& files, bool inNewTab) {
    if (files.empty()) {
        return;
    }

    if (auto path = files.front().parent()) {
        if (!inNewTab) {
            // Open in new window
            auto* win = new MainWindow(path);
            win->show();
            if (auto* page = win->currentPage()) {
                page->setFilesToSelect(files);
            }
        } else {
            // Open in new tab
            auto* newPage = new TabPage(this);
            addTabWithPage(newPage, activeViewFrame_, std::move(path));
            newPage->setFilesToSelect(files);
        }
    }
}

}  // namespace PCManFM
