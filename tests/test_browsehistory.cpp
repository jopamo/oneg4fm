/*
 * Unit tests for Fm::BrowseHistory behavior.
 */

#include <QObject>
#include <QTest>
#include <libfm-qt6/browsehistory.h>

namespace {

Fm::FilePath makePath(const char* path) {
    return Fm::FilePath::fromLocalPath(path);
}

}  // namespace

class TestBrowseHistory : public QObject {
    Q_OBJECT

   private Q_SLOTS:
    void addPrunesForwardHistory();
    void addRespectsMaxCount();
    void setMaxCountShrinksAndKeepsIndexValid();
    void setMaxCountClampsToPositiveValue();
};

void TestBrowseHistory::addPrunesForwardHistory() {
    Fm::BrowseHistory history;
    const Fm::FilePath pathA = makePath("/tmp/pcmanfm-qt-history-a");
    const Fm::FilePath pathB = makePath("/tmp/pcmanfm-qt-history-b");
    const Fm::FilePath pathC = makePath("/tmp/pcmanfm-qt-history-c");
    const Fm::FilePath pathD = makePath("/tmp/pcmanfm-qt-history-d");

    history.add(pathA, 1);
    history.add(pathB, 2);
    history.add(pathC, 3);

    history.setCurrentIndex(1);  // pathB
    history.add(pathD, 4);       // should drop pathC

    QCOMPARE(static_cast<int>(history.size()), 3);
    QVERIFY(history.at(0).path() == pathA);
    QVERIFY(history.at(1).path() == pathB);
    QVERIFY(history.at(2).path() == pathD);
    QVERIFY(history.currentPath() == pathD);
    QCOMPARE(history.currentScrollPos(), 4);
    QVERIFY(!history.canForward());
    QVERIFY(history.canBackward());
}

void TestBrowseHistory::addRespectsMaxCount() {
    Fm::BrowseHistory history;
    history.setMaxCount(3);

    const Fm::FilePath pathA = makePath("/tmp/pcmanfm-qt-history-max-a");
    const Fm::FilePath pathB = makePath("/tmp/pcmanfm-qt-history-max-b");
    const Fm::FilePath pathC = makePath("/tmp/pcmanfm-qt-history-max-c");
    const Fm::FilePath pathD = makePath("/tmp/pcmanfm-qt-history-max-d");

    history.add(pathA);
    history.add(pathB);
    history.add(pathC);
    history.add(pathD);

    QCOMPARE(static_cast<int>(history.size()), 3);
    QVERIFY(history.at(0).path() == pathB);
    QVERIFY(history.at(1).path() == pathC);
    QVERIFY(history.at(2).path() == pathD);
    QVERIFY(history.currentPath() == pathD);
}

void TestBrowseHistory::setMaxCountShrinksAndKeepsIndexValid() {
    Fm::BrowseHistory history;

    const Fm::FilePath pathA = makePath("/tmp/pcmanfm-qt-history-shrink-a");
    const Fm::FilePath pathB = makePath("/tmp/pcmanfm-qt-history-shrink-b");
    const Fm::FilePath pathC = makePath("/tmp/pcmanfm-qt-history-shrink-c");
    const Fm::FilePath pathD = makePath("/tmp/pcmanfm-qt-history-shrink-d");
    const Fm::FilePath pathE = makePath("/tmp/pcmanfm-qt-history-shrink-e");

    history.add(pathA);
    history.add(pathB);
    history.add(pathC);
    history.add(pathD);
    history.add(pathE);

    history.setCurrentIndex(2);  // pathC
    history.setMaxCount(3);

    QCOMPARE(history.maxCount(), 3);
    QCOMPARE(static_cast<int>(history.size()), 3);
    QVERIFY(history.at(0).path() == pathC);
    QVERIFY(history.at(1).path() == pathD);
    QVERIFY(history.at(2).path() == pathE);
    QVERIFY(history.currentPath() == pathC);
    QVERIFY(history.canForward());
    QVERIFY(!history.canBackward());
}

void TestBrowseHistory::setMaxCountClampsToPositiveValue() {
    Fm::BrowseHistory history;
    history.setMaxCount(0);

    const Fm::FilePath pathA = makePath("/tmp/pcmanfm-qt-history-zero-a");
    const Fm::FilePath pathB = makePath("/tmp/pcmanfm-qt-history-zero-b");

    QCOMPARE(history.maxCount(), 1);
    history.add(pathA);
    history.add(pathB);

    QCOMPARE(static_cast<int>(history.size()), 1);
    QVERIFY(history.currentPath() == pathB);
    QVERIFY(!history.canBackward());
    QVERIFY(!history.canForward());
}

QTEST_APPLESS_MAIN(TestBrowseHistory)
#include "test_browsehistory.moc"
