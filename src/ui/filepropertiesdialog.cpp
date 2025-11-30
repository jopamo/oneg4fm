/*
 * Qt-based file properties dialog for PCManFM-Qt
 * pcmanfm-qt/src/ui/filepropertiesdialog.cpp
 */

#include "filepropertiesdialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileIconProvider>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

FilePropertiesDialog::FilePropertiesDialog(const QList<std::shared_ptr<IFileInfo>>& fileInfos, QWidget* parent)
    : QDialog(parent), m_fileInfos(fileInfos) {
    setupUI();
    populateFileInfo();
}

FilePropertiesDialog::FilePropertiesDialog(std::shared_ptr<IFileInfo> fileInfo, QWidget* parent) : QDialog(parent) {
    m_fileInfos.append(fileInfo);
    setupUI();
    populateFileInfo();
}

void FilePropertiesDialog::setupUI() {
    setWindowTitle(tr("Properties"));
    setMinimumSize(400, 300);

    auto mainLayout = new QVBoxLayout(this);

    // Icon and basic info
    auto topLayout = new QHBoxLayout();
    m_iconLabel = new QLabel();
    m_iconLabel->setFixedSize(64, 64);
    topLayout->addWidget(m_iconLabel);

    auto infoLayout = new QVBoxLayout();
    m_nameLabel = new QLabel();
    m_nameLabel->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    infoLayout->addWidget(m_nameLabel);

    m_typeLabel = new QLabel();
    infoLayout->addWidget(m_typeLabel);
    topLayout->addLayout(infoLayout);

    mainLayout->addLayout(topLayout);

    // Tab widget for detailed properties
    m_tabWidget = new QTabWidget();
    setupGeneralTab();
    setupPermissionsTab();
    mainLayout->addWidget(m_tabWidget);

    // Buttons
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    m_applyButton = buttonBox->button(QDialogButtonBox::Apply);
    m_cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    connect(m_applyButton, &QPushButton::clicked, this, [this]() { onApplyClicked(); });
    connect(m_cancelButton, &QPushButton::clicked, this, [this]() { onCancelClicked(); });

    mainLayout->addWidget(buttonBox);
}

void FilePropertiesDialog::populateFileInfo() {
    if (m_fileInfos.isEmpty()) {
        return;
    }

    auto fileInfo = m_fileInfos.first();

    // Set icon
    QPixmap iconPixmap = fileInfo->icon().pixmap(64, 64);
    m_iconLabel->setPixmap(iconPixmap);

    // Set basic info
    if (m_fileInfos.size() == 1) {
        m_nameLabel->setText(fileInfo->displayName());
        m_typeLabel->setText(fileInfo->mimeType());
    } else {
        m_nameLabel->setText(tr("%1 items").arg(m_fileInfos.size()));
        m_typeLabel->setText(tr("Multiple items"));
    }
}

void FilePropertiesDialog::setupGeneralTab() {
    auto generalTab = new QWidget();
    auto layout = new QFormLayout(generalTab);

    m_sizeLabel = new QLabel();
    m_locationLabel = new QLabel();
    m_modifiedLabel = new QLabel();

    layout->addRow(tr("Size:"), m_sizeLabel);
    layout->addRow(tr("Location:"), m_locationLabel);
    layout->addRow(tr("Modified:"), m_modifiedLabel);

    // Populate with actual data
    if (!m_fileInfos.isEmpty()) {
        auto fileInfo = m_fileInfos.first();
        if (m_fileInfos.size() == 1) {
            m_sizeLabel->setText(QString::number(fileInfo->size()));
            m_locationLabel->setText(QFileInfo(fileInfo->path()).absolutePath());
            m_modifiedLabel->setText(fileInfo->lastModified().toString());
        } else {
            qint64 totalSize = 0;
            for (const auto& info : m_fileInfos) {
                totalSize += info->size();
            }
            m_sizeLabel->setText(QString::number(totalSize));
            m_locationLabel->setText(tr("Multiple locations"));
            m_modifiedLabel->setText(tr("Various"));
        }
    }

    m_tabWidget->addTab(generalTab, tr("General"));
}

void FilePropertiesDialog::setupPermissionsTab() {
    auto permissionsTab = new QWidget();
    auto layout = new QFormLayout(permissionsTab);

    // TODO: Implement permissions editing
    auto permissionsLabel = new QLabel(tr("Permissions editing not yet implemented"));
    layout->addRow(permissionsLabel);

    m_tabWidget->addTab(permissionsTab, tr("Permissions"));
}

void FilePropertiesDialog::onApplyClicked() {
    // TODO: Implement applying changes (permissions, etc.)
    accept();
}

void FilePropertiesDialog::onCancelClicked() { reject(); }