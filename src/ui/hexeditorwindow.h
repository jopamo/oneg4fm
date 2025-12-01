/*
 * Hex editor window
 * src/ui/hexeditorwindow.h
 */

#ifndef PCMANFM_HEXEDITORWINDOW_H
#define PCMANFM_HEXEDITORWINDOW_H

#include <QMainWindow>
#include <QPointer>

#include <memory>

#include "hexdocument.h"
#include "hexeditorview.h"

class QTimer;
class QLabel;

namespace PCManFM {

class HexEditorWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit HexEditorWindow(QWidget* parent = nullptr);
    ~HexEditorWindow() override;

    bool openFile(const QString& path, QString& errorOut);

   protected:
    void closeEvent(QCloseEvent* event) override;

   private:
    void setupUi();
    void updateWindowTitle();
    void updateStatus(std::uint64_t offset, int byteValue, bool hasSelection);
    void updateActionStates(bool hasSelection);
    bool promptToSave();
    void doSave(bool saveAs);
    void performFind(bool forward);
    void performReplace(bool replaceAll);
    void jumpToModified(bool forward);
    void checkExternalChanges();
    QByteArray parseSearchInput(const QString& input, bool& okOut) const;
    QByteArray promptForPattern(const QString& title, bool& okOut);
    QByteArray promptForReplace(bool& okOut);
    void goToOffset();

    std::unique_ptr<HexDocument> doc_;
    QPointer<HexEditorView> view_;
    QTimer* changeTimer_ = nullptr;
    bool suppressExternalPrompt_ = false;
    quint64 lastExternalFingerprint_ = 0;

    QAction* saveAction_ = nullptr;
    QAction* saveAsAction_ = nullptr;
    QAction* undoAction_ = nullptr;
    QAction* redoAction_ = nullptr;
    QAction* copyAction_ = nullptr;
    QAction* copyHexAction_ = nullptr;
    QAction* pasteAction_ = nullptr;
    QAction* deleteAction_ = nullptr;
    QAction* findAction_ = nullptr;
    QAction* findNextAction_ = nullptr;
    QAction* findPrevAction_ = nullptr;
    QAction* findAllAction_ = nullptr;
    QAction* replaceAction_ = nullptr;
    QAction* replaceAllAction_ = nullptr;
    QAction* gotoAction_ = nullptr;
    QAction* insertToggleAction_ = nullptr;
    QAction* nextModifiedAction_ = nullptr;
    QAction* prevModifiedAction_ = nullptr;

    QByteArray lastSearch_;
    QByteArray lastReplace_;

    QLabel* modeLabel_ = nullptr;
    QLabel* modifiedLabel_ = nullptr;
};

}  // namespace PCManFM

#endif  // PCMANFM_HEXEDITORWINDOW_H
