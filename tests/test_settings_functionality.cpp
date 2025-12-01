/*
 * Unit tests for Settings class functionality
 * tests/test_settings_functionality.cpp
 */

#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>
#include <QTest>

// Include the settings header
#include "settings.h"

class TestSettingsFunctionality : public QObject {
    Q_OBJECT

   private slots:
    void testSettingsInitialization();
    void testSettingsProfileDir();
    void testSettingsStringConversions();
    void testSettingsLoadSave();
};

void TestSettingsFunctionality::testSettingsInitialization() {
    // Test that Settings can be constructed
    PCManFM::Settings settings;

    // Test default values
    QVERIFY(!settings.singleWindowMode());
    QVERIFY(settings.mountOnStartup());
    QVERIFY(settings.mountRemovable());
    QVERIFY(settings.autoRun());
    QVERIFY(settings.alwaysShowTabs());
    QVERIFY(settings.showTabClose());
    QVERIFY(settings.rememberWindowSize());
    QVERIFY(settings.isSidePaneVisible());
    QVERIFY(settings.showMenuBar());
    QVERIFY(!settings.splitView());
    QVERIFY(!settings.showHidden());
    QVERIFY(settings.sortFolderFirst());
    QVERIFY(!settings.sortHiddenLast());
    QVERIFY(!settings.sortCaseSensitive());
    QVERIFY(!settings.showFilter());
    QVERIFY(settings.pathBarButtons());
    QVERIFY(!settings.singleClick());
    QVERIFY(settings.useTrash());
    QVERIFY(settings.confirmDelete());
    QVERIFY(!settings.noUsbTrash());
    QVERIFY(!settings.confirmTrash());
    QVERIFY(!settings.quickExec());
    QVERIFY(!settings.selectNewFiles());
    QVERIFY(settings.showThumbnails());
    QVERIFY(!settings.siUnit());
    QVERIFY(!settings.backupAsHidden());
    QVERIFY(settings.showFullNames());
    QVERIFY(settings.shadowHidden());
    QVERIFY(!settings.noItemTooltip());
    QVERIFY(settings.scrollPerPixel());
    QVERIFY(!settings.onlyUserTemplates());
    QVERIFY(!settings.templateTypeOnce());
    QVERIFY(!settings.templateRunApp());
    QVERIFY(!settings.openWithDefaultFileManager());
    QVERIFY(!settings.allSticky());
    QVERIFY(!settings.searchNameCaseInsensitive());
    QVERIFY(!settings.searchContentCaseInsensitive());
    QVERIFY(settings.searchNameRegexp());
    QVERIFY(settings.searchContentRegexp());
    QVERIFY(!settings.searchRecursive());
    QVERIFY(!settings.searchhHidden());
    QCOMPARE(settings.maxSearchHistory(), 0);
}

void TestSettingsFunctionality::testSettingsProfileDir() {
    // Test profile directory resolution
    PCManFM::Settings settings;

    QString profileDir = settings.profileDir("default");
    QVERIFY(!profileDir.isEmpty());

    // Should contain "pcmanfm-qt" and the profile name
    QVERIFY(profileDir.contains("pcmanfm-qt"));
    QVERIFY(profileDir.contains("default"));

    // Test with fallback
    QString profileDirWithFallback = settings.profileDir("test-profile", true);
    QVERIFY(!profileDirWithFallback.isEmpty());
}

void TestSettingsFunctionality::testSettingsStringConversions() {
    // Test string conversion utilities

    // View mode conversions
    const char* iconMode = PCManFM::Settings::viewModeToString(Fm::FolderView::IconMode);
    QCOMPARE(QString(iconMode), QString("icon"));

    const char* compactMode = PCManFM::Settings::viewModeToString(Fm::FolderView::CompactMode);
    QCOMPARE(QString(compactMode), QString("compact"));

    const char* detailedMode = PCManFM::Settings::viewModeToString(Fm::FolderView::DetailedListMode);
    QCOMPARE(QString(detailedMode), QString("detailed"));

    const char* thumbnailMode = PCManFM::Settings::viewModeToString(Fm::FolderView::ThumbnailMode);
    QCOMPARE(QString(thumbnailMode), QString("thumbnail"));

    // Sort order conversions
    const char* ascending = PCManFM::Settings::sortOrderToString(Qt::AscendingOrder);
    QCOMPARE(QString(ascending), QString("ascending"));

    const char* descending = PCManFM::Settings::sortOrderToString(Qt::DescendingOrder);
    QCOMPARE(QString(descending), QString("descending"));

    // Sort column conversions
    const char* nameColumn = PCManFM::Settings::sortColumnToString(Fm::FolderModel::ColumnFileName);
    QCOMPARE(QString(nameColumn), QString("name"));

    const char* sizeColumn = PCManFM::Settings::sortColumnToString(Fm::FolderModel::ColumnFileSize);
    QCOMPARE(QString(sizeColumn), QString("size"));

    const char* mtimeColumn = PCManFM::Settings::sortColumnToString(Fm::FolderModel::ColumnFileMTime);
    QCOMPARE(QString(mtimeColumn), QString("mtime"));

    // Side pane mode conversions
    const char* placesMode = PCManFM::Settings::sidePaneModeToString(Fm::SidePane::ModePlaces);
    QCOMPARE(QString(placesMode), QString("places"));

    const char* dirTreeMode = PCManFM::Settings::sidePaneModeToString(Fm::SidePane::ModeDirTree);
    QCOMPARE(QString(dirTreeMode), QString("dirtree"));

    const char* noneMode = PCManFM::Settings::sidePaneModeToString(Fm::SidePane::ModeNone);
    QCOMPARE(QString(noneMode), QString("none"));
}

void TestSettingsFunctionality::testSettingsLoadSave() {
    // Create a temporary directory for test settings
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Set up environment to use temporary directory for config
    qputenv("XDG_CONFIG_HOME", tempDir.path().toUtf8());

    PCManFM::Settings settings;

    // Test loading non-existent profile (should return false)
    bool loadResult = settings.load("test-profile");
    QVERIFY(!loadResult);

    // Test saving settings
    bool saveResult = settings.save("test-profile");
    QVERIFY(saveResult);

    // Verify settings file was created
    QString settingsPath = settings.profileDir("test-profile") + QStringLiteral("/settings.conf");
    QVERIFY(QFile::exists(settingsPath));

    // Test loading the saved settings
    PCManFM::Settings loadedSettings;
    bool reloadResult = loadedSettings.load("test-profile");
    QVERIFY(reloadResult);

    // Verify loaded settings match original defaults
    QCOMPARE(loadedSettings.singleWindowMode(), settings.singleWindowMode());
    QCOMPARE(loadedSettings.mountOnStartup(), settings.mountOnStartup());
    QCOMPARE(loadedSettings.mountRemovable(), settings.mountRemovable());
    QCOMPARE(loadedSettings.autoRun(), settings.autoRun());
    QCOMPARE(loadedSettings.alwaysShowTabs(), settings.alwaysShowTabs());
    QCOMPARE(loadedSettings.showTabClose(), settings.showTabClose());
    QCOMPARE(loadedSettings.rememberWindowSize(), settings.rememberWindowSize());
    QCOMPARE(loadedSettings.isSidePaneVisible(), settings.isSidePaneVisible());
    QCOMPARE(loadedSettings.showMenuBar(), settings.showMenuBar());
    QCOMPARE(loadedSettings.splitView(), settings.splitView());
    QCOMPARE(loadedSettings.showHidden(), settings.showHidden());
    QCOMPARE(loadedSettings.sortFolderFirst(), settings.sortFolderFirst());
    QCOMPARE(loadedSettings.sortHiddenLast(), settings.sortHiddenLast());
    QCOMPARE(loadedSettings.sortCaseSensitive(), settings.sortCaseSensitive());
    QCOMPARE(loadedSettings.showFilter(), settings.showFilter());
    QCOMPARE(loadedSettings.pathBarButtons(), settings.pathBarButtons());
    QCOMPARE(loadedSettings.singleClick(), settings.singleClick());
    QCOMPARE(loadedSettings.useTrash(), settings.useTrash());
    QCOMPARE(loadedSettings.confirmDelete(), settings.confirmDelete());
    QCOMPARE(loadedSettings.noUsbTrash(), settings.noUsbTrash());
    QCOMPARE(loadedSettings.confirmTrash(), settings.confirmTrash());
    QCOMPARE(loadedSettings.quickExec(), settings.quickExec());
    QCOMPARE(loadedSettings.selectNewFiles(), settings.selectNewFiles());
    QCOMPARE(loadedSettings.showThumbnails(), settings.showThumbnails());
    QCOMPARE(loadedSettings.siUnit(), settings.siUnit());
    QCOMPARE(loadedSettings.backupAsHidden(), settings.backupAsHidden());
    QCOMPARE(loadedSettings.showFullNames(), settings.showFullNames());
    QCOMPARE(loadedSettings.shadowHidden(), settings.shadowHidden());
    QCOMPARE(loadedSettings.noItemTooltip(), settings.noItemTooltip());
    QCOMPARE(loadedSettings.scrollPerPixel(), settings.scrollPerPixel());
    QCOMPARE(loadedSettings.onlyUserTemplates(), settings.onlyUserTemplates());
    QCOMPARE(loadedSettings.templateTypeOnce(), settings.templateTypeOnce());
    QCOMPARE(loadedSettings.templateRunApp(), settings.templateRunApp());
    QCOMPARE(loadedSettings.openWithDefaultFileManager(), settings.openWithDefaultFileManager());
    QCOMPARE(loadedSettings.allSticky(), settings.allSticky());
    QCOMPARE(loadedSettings.searchNameCaseInsensitive(), settings.searchNameCaseInsensitive());
    QCOMPARE(loadedSettings.searchContentCaseInsensitive(), settings.searchContentCaseInsensitive());
    QCOMPARE(loadedSettings.searchNameRegexp(), settings.searchNameRegexp());
    QCOMPARE(loadedSettings.searchContentRegexp(), settings.searchContentRegexp());
    QCOMPARE(loadedSettings.searchRecursive(), settings.searchRecursive());
    QCOMPARE(loadedSettings.searchhHidden(), settings.searchhHidden());
    QCOMPARE(loadedSettings.maxSearchHistory(), settings.maxSearchHistory());
}

QTEST_MAIN(TestSettingsFunctionality)
#include "test_settings_functionality.moc"
