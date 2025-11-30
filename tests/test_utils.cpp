/*
 * Unit tests for general utility functions and core logic
 * tests/test_utils.cpp
 */

#include <QObject>
#include <QString>
#include <QTest>

// Include headers for utility functions
#include "settings.h"

using namespace PCManFM;

class TestUtils : public QObject {
    Q_OBJECT

   private slots:
    void testViewModeToString();
    void testSortOrderToString();
    void testSortColumnToString();
    void testSidePaneModeToString();
    void testStringConversionRoundtrip();
};

void TestUtils::testViewModeToString() {
    // Test view mode to string conversion
    QCOMPARE(QString(viewModeToString(Fm::FolderView::IconMode)), QString("icon"));
    QCOMPARE(QString(viewModeToString(Fm::FolderView::CompactMode)), QString("compact"));
    QCOMPARE(QString(viewModeToString(Fm::FolderView::DetailedListMode)), QString("detailed"));
    QCOMPARE(QString(viewModeToString(Fm::FolderView::ThumbnailMode)), QString("thumbnail"));
}

void TestUtils::testSortOrderToString() {
    // Test sort order to string conversion
    QCOMPARE(QString(sortOrderToString(Qt::AscendingOrder)), QString("ascending"));
    QCOMPARE(QString(sortOrderToString(Qt::DescendingOrder)), QString("descending"));
}

void TestUtils::testSortColumnToString() {
    // Test sort column to string conversion
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileName)), QString("name"));
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileSize)), QString("size"));
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileType)), QString("type"));
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileMTime)), QString("mtime"));
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileCrTime)), QString("ctime"));
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileDTime)), QString("dtime"));
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileOwner)), QString("owner"));
    QCOMPARE(QString(sortColumnToString(Fm::FolderModel::ColumnFileGroup)), QString("group"));
}

void TestUtils::testSidePaneModeToString() {
    // Test side pane mode to string conversion
    QCOMPARE(QString(sidePaneModeToString(Fm::SidePane::Places)), QString("places"));
    QCOMPARE(QString(sidePaneModeToString(Fm::SidePane::DirectoryTree)), QString("directory_tree"));
}

void TestUtils::testStringConversionRoundtrip() {
    // Test that string conversions are reversible

    // View modes
    for (auto mode : {Fm::FolderView::IconMode, Fm::FolderView::CompactMode, Fm::FolderView::DetailedListMode,
                      Fm::FolderView::ThumbnailMode}) {
        QString str = viewModeToString(mode);
        Fm::FolderView::ViewMode converted = viewModeFromString(str);
        QCOMPARE(converted, mode);
    }

    // Sort orders
    for (auto order : {Qt::AscendingOrder, Qt::DescendingOrder}) {
        QString str = sortOrderToString(order);
        Qt::SortOrder converted = sortOrderFromString(str);
        QCOMPARE(converted, order);
    }

    // Sort columns
    for (auto column :
         {Fm::FolderModel::ColumnFileName, Fm::FolderModel::ColumnFileSize, Fm::FolderModel::ColumnFileType,
          Fm::FolderModel::ColumnFileMTime, Fm::FolderModel::ColumnFileCrTime, Fm::FolderModel::ColumnFileDTime,
          Fm::FolderModel::ColumnFileOwner, Fm::FolderModel::ColumnFileGroup}) {
        QString str = sortColumnToString(column);
        Fm::FolderModel::ColumnId converted = sortColumnFromString(str);
        QCOMPARE(converted, column);
    }

    // Side pane modes
    for (auto mode : {Fm::SidePane::Places, Fm::SidePane::DirectoryTree}) {
        QString str = sidePaneModeToString(mode);
        Fm::SidePane::Mode converted = sidePaneModeFromString(str);
        QCOMPARE(converted, mode);
    }
}

QTEST_MAIN(TestUtils)
#include "test_utils.moc"