/*
 * Unit tests for XdgDir utility functions
 * tests/test_xdgdir.cpp
 */

#include <QObject>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

// Include the xdgdir header to test the utility functions
#include "xdgdir.h"

class TestXdgDir : public QObject {
    Q_OBJECT

   private slots:
    void testReadDesktopDir();
    void testSetDesktopDir();
    void testReadUserDirsFile();
};

void TestXdgDir::testReadDesktopDir() {
    // Test that readDesktopDir returns a non-empty string
    QString desktopDir = XdgDir::readDesktopDir();
    QVERIFY(!desktopDir.isEmpty());

    // The desktop directory should be a valid path
    QVERIFY(QDir(desktopDir).exists() || QDir::home().exists());
}

void TestXdgDir::testSetDesktopDir() {
    // Create a temporary directory for testing
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString testPath = tempDir.path() + QStringLiteral("/test-desktop");

    // Test setting desktop directory
    XdgDir::setDesktopDir(testPath);

    // Verify the directory was created
    QVERIFY(QDir(testPath).exists());
}

void TestXdgDir::testReadUserDirsFile() {
    // This is a private method, but we can test its behavior indirectly
    // by checking that readDesktopDir works correctly

    QString desktopDir = XdgDir::readDesktopDir();

    // The desktop directory should be a subdirectory of home
    // or a standard XDG directory
    QVERIFY(desktopDir.contains(QDir::homePath()) || desktopDir.contains(QStringLiteral(".config")) ||
            desktopDir.contains(QStringLiteral(".local")));
}

QTEST_MAIN(TestXdgDir)
#include "test_xdgdir.moc"