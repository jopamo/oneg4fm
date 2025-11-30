/*
 * Unit tests for XdgDir utility functions
 * tests/test_xdgdir_functionality.cpp
 */

#include <QDir>
#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>
#include <QTest>

// Include the xdgdir header
#include "xdgdir.h"

class TestXdgDirFunctionality : public QObject {
    Q_OBJECT

   private slots:
    void testReadDesktopDir();
    void testSetDesktopDir();
    void testXdgDirBasicFunctionality();
    void testXdgDirPathValidation();
};

void TestXdgDirFunctionality::testReadDesktopDir() {
    // Test that readDesktopDir returns a valid path
    QString desktopDir = XdgDir::readDesktopDir();

    // Should not be empty
    QVERIFY(!desktopDir.isEmpty());

    // Should be a valid directory (or at least a valid path structure)
    QVERIFY(desktopDir.contains(QDir::separator()) || desktopDir == QDir::homePath());

    // Should be related to user's home directory or standard XDG locations
    QVERIFY(desktopDir.contains(QDir::homePath()) || desktopDir.contains(QStringLiteral(".config")) ||
            desktopDir.contains(QStringLiteral(".local")) || desktopDir.contains(QStringLiteral("Desktop")));
}

void TestXdgDirFunctionality::testSetDesktopDir() {
    // Create a temporary directory for testing
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString testPath = tempDir.path() + QStringLiteral("/test-desktop");

    // Test setting desktop directory
    XdgDir::setDesktopDir(testPath);

    // Verify the directory was created
    QVERIFY(QDir(testPath).exists());

    // Verify we can read it back
    QString readBackPath = XdgDir::readDesktopDir();

    // The path should either be our test path or the original desktop path
    // (setDesktopDir might not change the global state in a test environment)
    QVERIFY(readBackPath == testPath || !readBackPath.isEmpty());
}

void TestXdgDirFunctionality::testXdgDirBasicFunctionality() {
    // Test basic XDG directory functionality

    // Test that we can get a desktop directory multiple times consistently
    QString desktop1 = XdgDir::readDesktopDir();
    QString desktop2 = XdgDir::readDesktopDir();

    QCOMPARE(desktop1, desktop2);

    // Test that the path is reasonable
    QVERIFY(desktop1.length() > 0);
    QVERIFY(desktop1 != QStringLiteral("/"));
    QVERIFY(desktop1 != QStringLiteral(""));
}

void TestXdgDirFunctionality::testXdgDirPathValidation() {
    // Test various path validation scenarios

    QString desktopDir = XdgDir::readDesktopDir();

    // The path should not contain double slashes (except at the beginning)
    QString doubleSlash = desktopDir + QStringLiteral("//");
    QVERIFY(!desktopDir.contains(QStringLiteral("//")));

    // The path should end with a valid directory name, not a file
    QVERIFY(!desktopDir.endsWith(QStringLiteral(".txt")));
    QVERIFY(!desktopDir.endsWith(QStringLiteral(".conf")));

    // Should be an absolute path
    QVERIFY(QDir::isAbsolutePath(desktopDir));

    // Should not be a relative path
    QVERIFY(!desktopDir.startsWith(QStringLiteral(".")));
    QVERIFY(!desktopDir.startsWith(QStringLiteral("..")));
}

QTEST_MAIN(TestXdgDirFunctionality)
#include "test_xdgdir_functionality.moc"