#include "bulkrename.h"

#include <libfm-qt6/utilities.h>

#include <QLocale>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QRegularExpression>
#include <QTimer>

namespace PCManFM {

BulkRenameDialog::BulkRenameDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
    ui.setupUi(this);
    ui.lineEdit->setFocus();

    connect(ui.buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, this, &QDialog::accept);
    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked, this, &QDialog::reject);

    // make the groupboxes mutually exclusive
    connect(ui.serialGroupBox, &QGroupBox::clicked, this, [this](bool checked) {
        if (!checked) {
            ui.serialGroupBox->setChecked(true);
        }
        ui.replaceGroupBox->setChecked(false);
        ui.caseGroupBox->setChecked(false);
    });

    connect(ui.replaceGroupBox, &QGroupBox::clicked, this, [this](bool checked) {
        if (!checked) {
            ui.replaceGroupBox->setChecked(true);
        }
        ui.serialGroupBox->setChecked(false);
        ui.caseGroupBox->setChecked(false);
    });

    connect(ui.caseGroupBox, &QGroupBox::clicked, this, [this](bool checked) {
        if (!checked) {
            ui.caseGroupBox->setChecked(true);
        }
        ui.serialGroupBox->setChecked(false);
        ui.replaceGroupBox->setChecked(false);
    });

    resize(minimumSize());
    setMaximumHeight(minimumSizeHint().height());
}

void BulkRenameDialog::setState(const QString& baseName, const QString& findStr, const QString& replaceStr,
                                bool replacement, bool caseChange, bool zeroPadding, bool respectLocale, bool regex,
                                bool toUpperCase, int start, Qt::CaseSensitivity cs) {
    if (!baseName.isEmpty()) {
        ui.lineEdit->setText(baseName);
    }

    ui.spinBox->setValue(start);
    ui.zeroBox->setChecked(zeroPadding);
    ui.localeBox->setChecked(respectLocale);

    if (replacement || caseChange) {
        ui.serialGroupBox->setChecked(false);
        if (replacement) {
            ui.replaceGroupBox->setChecked(true);
        } else {
            ui.caseGroupBox->setChecked(true);
        }
    }

    ui.findLineEdit->setText(findStr);
    ui.replaceLineEdit->setText(replaceStr);
    ui.caseBox->setChecked(cs == Qt::CaseSensitive);
    ui.regexBox->setChecked(regex);

    if (toUpperCase) {
        ui.upperCaseButton->setChecked(true);
    } else {
        ui.lowerCaseButton->setChecked(true);
    }
}

void BulkRenameDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);

    if (ui.lineEdit->text().endsWith(QLatin1Char('#'))) {
        QTimer::singleShot(0, this, [this]() { ui.lineEdit->setSelection(0, ui.lineEdit->text().size() - 1); });
    }
}

// helper to resolve a usable name from FileInfo, respecting edit name and encoding
static QString effectiveFileName(const std::shared_ptr<const Fm::FileInfo>& file) {
    QString fileName = QString::fromUtf8(g_file_info_get_edit_name(file->gFileInfo().get()));

    if (fileName.isEmpty()) {
        fileName = QString::fromStdString(file->name());
    }

    return fileName;
}

BulkRenamer::BulkRenamer(const Fm::FileInfoList& files, QWidget* parent) {
    if (files.size() <= 1) {
        return;
    }

    bool replacement = false;
    bool caseChange = false;
    QString baseName;
    QString findStr;
    QString replaceStr;
    int start = 0;
    bool zeroPadding = false;
    bool respectLocale = false;
    bool regex = false;
    bool toUpperCase = true;
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QLocale locale;
    bool showDlg = true;

    while (showDlg) {
        BulkRenameDialog dlg(parent);
        dlg.setState(baseName, findStr, replaceStr, replacement, caseChange, zeroPadding, respectLocale, regex,
                     toUpperCase, start, cs);

        if (dlg.exec() != QDialog::Accepted) {
            return;
        }

        baseName = dlg.getBaseName();
        start = dlg.getStart();
        zeroPadding = dlg.getZeroPadding();
        respectLocale = dlg.getRespectLocale();
        locale = dlg.locale();

        replacement = dlg.getReplace();
        findStr = dlg.getFindStr();
        replaceStr = dlg.getReplaceStr();
        cs = dlg.getCase();
        regex = dlg.getRegex();

        caseChange = dlg.getCaseChange();
        toUpperCase = dlg.getUpperCase();
        locale = dlg.locale();

        if (replacement) {
            showDlg = !renameByReplacing(files, findStr, replaceStr, cs, regex, parent);
        } else if (caseChange) {
            showDlg = !renameByChangingCase(files, locale, toUpperCase, parent);
        } else {
            showDlg = !rename(files, baseName, locale, start, zeroPadding, respectLocale, parent);
        }
    }
}

bool BulkRenamer::rename(const Fm::FileInfoList& files, QString& baseName, const QLocale& locale, int start,
                         bool zeroPadding, bool respectLocale, QWidget* parent) {
    const int numSpace = zeroPadding ? QString::number(start + int(files.size())).size() : 0;

    const QChar zero = respectLocale ? (!locale.zeroDigit().isEmpty() ? locale.zeroDigit().at(0) : QLatin1Char('0'))
                                     : QLatin1Char('0');

    const QString specifier = respectLocale ? QStringLiteral("%L1") : QStringLiteral("%1");

    if (!baseName.contains(QLatin1Char('#'))) {
        int end = baseName.lastIndexOf(QLatin1Char('.'));
        if (end == -1) {
            end = baseName.size();
        }
        baseName.insert(end, QLatin1Char('#'));
    }

    QProgressDialog progress(QObject::tr("Renaming files..."), QObject::tr("Abort"), 0, files.size(), parent);
    progress.setWindowModality(Qt::WindowModal);

    int i = 0;
    int failed = 0;

    const QRegularExpression extension(QStringLiteral("\\.[^.#]+$"));
    const bool noExtension = (baseName.indexOf(extension) == -1);

    for (auto& file : files) {
        progress.setValue(i);

        if (progress.wasCanceled()) {
            progress.close();
            QMessageBox::warning(parent, QObject::tr("Warning"), QObject::tr("Renaming is aborted."));
            return true;
        }

        QString fileName = effectiveFileName(file);
        QString newName = baseName;

        if (noExtension) {
            QRegularExpressionMatch match;
            if (fileName.indexOf(extension, 0, &match) > -1) {
                newName += match.captured();
            }
        }

        newName.replace(QLatin1Char('#'), specifier.arg(start + i, numSpace, 10, zero));

        if (newName == fileName || !Fm::changeFileName(file->path(), newName, nullptr, false)) {
            ++failed;
        }

        ++i;
    }

    progress.setValue(i);

    if (failed == i) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("No file could be renamed."));
        return false;
    }

    if (failed > 0) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("Some files could not be renamed."));
    }

    return true;
}

bool BulkRenamer::renameByReplacing(const Fm::FileInfoList& files, const QString& findStr, const QString& replaceStr,
                                    Qt::CaseSensitivity cs, bool regex, QWidget* parent) {
    if (findStr.isEmpty()) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("Nothing to find."));
        return false;
    }

    QRegularExpression regexFind;
    if (regex) {
        regexFind = QRegularExpression(findStr, cs == Qt::CaseSensitive ? QRegularExpression::NoPatternOption
                                                                        : QRegularExpression::CaseInsensitiveOption);
        if (!regexFind.isValid()) {
            QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("Invalid regular expression."));
            return false;
        }
    }

    QProgressDialog progress(QObject::tr("Renaming files..."), QObject::tr("Abort"), 0, files.size(), parent);
    progress.setWindowModality(Qt::WindowModal);

    int i = 0;
    int failed = 0;

    for (auto& file : files) {
        progress.setValue(i);

        if (progress.wasCanceled()) {
            progress.close();
            QMessageBox::warning(parent, QObject::tr("Warning"), QObject::tr("Renaming is aborted."));
            return true;
        }

        QString fileName = effectiveFileName(file);
        QString newName = fileName;

        if (regex) {
            newName.replace(regexFind, replaceStr);
        } else {
            newName.replace(findStr, replaceStr, cs);
        }

        if (newName.isEmpty() || newName == fileName || !Fm::changeFileName(file->path(), newName, nullptr, false)) {
            ++failed;
        }

        ++i;
    }

    progress.setValue(i);

    if (failed == i) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("No file could be renamed."));
        return false;
    }

    if (failed > 0) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("Some files could not be renamed."));
    }

    return true;
}

bool BulkRenamer::renameByChangingCase(const Fm::FileInfoList& files, const QLocale& locale, bool toUpperCase,
                                       QWidget* parent) {
    QProgressDialog progress(QObject::tr("Renaming files..."), QObject::tr("Abort"), 0, files.size(), parent);
    progress.setWindowModality(Qt::WindowModal);

    int i = 0;
    int failed = 0;

    for (auto& file : files) {
        progress.setValue(i);

        if (progress.wasCanceled()) {
            progress.close();
            QMessageBox::warning(parent, QObject::tr("Warning"), QObject::tr("Renaming is aborted."));
            return true;
        }

        QString fileName = effectiveFileName(file);

        QString newName = toUpperCase ? locale.toUpper(fileName) : locale.toLower(fileName);

        if (newName.isEmpty() || newName == fileName || !Fm::changeFileName(file->path(), newName, nullptr, false)) {
            ++failed;
        }

        ++i;
    }

    progress.setValue(i);

    if (failed == i) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("No file could be renamed."));
        return false;
    }

    if (failed > 0) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("Some files could not be renamed."));
    }

    return true;
}

BulkRenamer::~BulkRenamer() = default;

}  // namespace PCManFM
