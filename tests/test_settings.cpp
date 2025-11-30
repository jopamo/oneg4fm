/*
 * Unit tests for Settings utility functions
 * tests/test_settings.cpp
 */

#include <QObject>
#include <QTest>

// Include the settings header to test the utility functions
#include "settings.h"

using namespace PCManFM;

class TestSettings : public QObject {
    Q_OBJECT

   private slots:
    void testBookmarkOpenMethodFromString();
    void testViewModeFromString();
    void testSortOrderFromString();
    void testSortColumnFromString();
    void testSidePaneModeFromString();
    void testFolderSettings();
};

void TestSettings::testBookmarkOpenMethodFromString() {
    // Test all valid bookmark open methods
    QCOMPARE(bookmarkOpenMethodFromString("new_tab"), OpenInNewTab);
    QCOMPARE(bookmarkOpenMethodFromString("new_window"), OpenInNewWindow);
    QCOMPARE(bookmarkOpenMethodFromString("last_window"), OpenInLastActiveWindow);

    // Test default case
    QCOMPARE(bookmarkOpenMethodFromString("invalid"), OpenInCurrentTab);
    QCOMPARE(bookmarkOpenMethodFromString(""), OpenInCurrentTab);
}

void TestSettings::testViewModeFromString() {
    // Test all valid view modes
    QCOMPARE(viewModeFromString("icon"), Fm::FolderView::IconMode);
    QCOMPARE(viewModeFromString("compact"), Fm::FolderView::CompactMode);
    QCOMPARE(viewModeFromString("detailed"), Fm::FolderView::DetailedListMode);
    QCOMPARE(viewModeFromString("thumbnail"), Fm::FolderView::ThumbnailMode);

    // Test default case
    QCOMPARE(viewModeFromString("invalid"), Fm::FolderView::IconMode);
    QCOMPARE(viewModeFromString(""), Fm::FolderView::IconMode);
}

void TestSettings::testSortOrderFromString() {
    // Test sort order parsing
    QCOMPARE(sortOrderFromString("ascending"), Qt::AscendingOrder);
    QCOMPARE(sortOrderFromString("descending"), Qt::DescendingOrder);

    // Test default case
    QCOMPARE(sortOrderFromString("invalid"), Qt::AscendingOrder);
    QCOMPARE(sortOrderFromString(""), Qt::AscendingOrder);
}

void TestSettings::testSortColumnFromString() {
    // Test sort column parsing
    QCOMPARE(sortColumnFromString("name"), Fm::FolderModel::ColumnFileName);
    QCOMPARE(sortColumnFromString("size"), Fm::FolderModel::ColumnFileSize);
    QCOMPARE(sortColumnFromString("type"), Fm::FolderModel::ColumnFileType);
    QCOMPARE(sortColumnFromString("mtime"), Fm::FolderModel::ColumnFileMTime);
    QCOMPARE(sortColumnFromString("ctime"), Fm::FolderModel::ColumnFileCrTime);
    QCOMPARE(sortColumnFromString("dtime"), Fm::FolderModel::ColumnFileDTime);
    QCOMPARE(sortColumnFromString("owner"), Fm::FolderModel::ColumnFileOwner);
    QCOMPARE(sortColumnFromString("group"), Fm::FolderModel::ColumnFileGroup);

    // Test default case
    QCOMPARE(sortColumnFromString("invalid"), Fm::FolderModel::ColumnFileName);
    QCOMPARE(sortColumnFromString(""), Fm::FolderModel::ColumnFileName);
}

void TestSettings::testSidePaneModeFromString() {
    // Test side pane mode parsing
    QCOMPARE(sidePaneModeFromString("places"), Fm::SidePane::Places);
    QCOMPARE(sidePaneModeFromString("directory_tree"), Fm::SidePane::DirectoryTree);

    // Test default case
    QCOMPARE(sidePaneModeFromString("invalid"), Fm::SidePane::Places);
    QCOMPARE(sidePaneModeFromString(""), Fm::SidePane::Places);
}

void TestSettings::testFolderSettings() {
    // Test FolderSettings class
    FolderSettings settings;

    // Test default values
    QVERIFY(!settings.isCustomized());
    QCOMPARE(settings.sortOrder(), Qt::AscendingOrder);
    QCOMPARE(settings.sortColumn(), Fm::FolderModel::ColumnFileName);
    QCOMPARE(settings.viewMode(), Fm::FolderView::IconMode);
    QVERIFY(!settings.showHidden());
    QVERIFY(settings.sortFolderFirst());
    QVERIFY(!settings.sortHiddenLast());
    QVERIFY(settings.sortCaseSensitive());
    QVERIFY(!settings.recursive());

    // Test setters
    settings.setCustomized(true);
    QVERIFY(settings.isCustomized());

    settings.setSortOrder(Qt::DescendingOrder);
    QCOMPARE(settings.sortOrder(), Qt::DescendingOrder);

    settings.setSortColumn(Fm::FolderModel::ColumnFileSize);
    QCOMPARE(settings.sortColumn(), Fm::FolderModel::ColumnFileSize);

    settings.setViewMode(Fm::FolderView::DetailedListMode);
    QCOMPARE(settings.viewMode(), Fm::FolderView::DetailedListMode);

    settings.setShowHidden(true);
    QVERIFY(settings.showHidden());

    settings.setSortFolderFirst(false);
    QVERIFY(!settings.sortFolderFirst());

    settings.setSortHiddenLast(true);
    QVERIFY(settings.sortHiddenLast());

    settings.setSortCaseSensitive(false);
    QVERIFY(!settings.sortCaseSensitive());

    settings.setRecursive(true);
    QVERIFY(settings.recursive());
}

QTEST_MAIN(TestSettings)
#include "test_settings.moc"