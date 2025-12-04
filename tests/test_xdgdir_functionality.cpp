/*
 * Unit tests for XdgDir utility functions
 * tests/test_xdgdir_functionality.cpp
 */

#include <QDir>
#include <QFile>
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
    void initTestCase();
    void testReadDesktopDir();
    void testSetDesktopDir();
    void testXdgDirBasicFunctionality();
    void testXdgDirPathValidation();

   private:
    QTemporaryDir configDir_;
    QString home_;
};

void TestXdgDirFunctionality::initTestCase() {
    QVERIFY(configDir_.isValid());
    home_ = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qputenv("XDG_CONFIG_HOME", configDir_.path().toUtf8());
    QFile::remove(configDir_.filePath(QStringLiteral("user-dirs.dirs")));
}

void TestXdgDirFunctionality::testReadDesktopDir() {
    const QString expectedDefault = home_ + QStringLiteral("/Desktop");

    QCOMPARE(XdgDir::readDesktopDir(), expectedDefault);
}

void TestXdgDirFunctionality::testSetDesktopDir() {
    const QString userDirsPath = configDir_.filePath(QStringLiteral("user-dirs.dirs"));
    const QString testPath = home_ + QStringLiteral("/test-desktop");

    XdgDir::setDesktopDir(testPath);

    QFile userDirs(userDirsPath);
    QVERIFY(userDirs.open(QIODevice::ReadOnly));
    const QString contents = QString::fromUtf8(userDirs.readAll());
    QVERIFY(contents.contains(QStringLiteral("XDG_DESKTOP_DIR=\"$HOME/test-desktop\"")));

    // Verify we can read it back
    QCOMPARE(XdgDir::readDesktopDir(), testPath);
}

void TestXdgDirFunctionality::testXdgDirBasicFunctionality() {
    const QString expectedPath = home_ + QStringLiteral("/basic-desktop");
    XdgDir::setDesktopDir(expectedPath);

    const QString desktop1 = XdgDir::readDesktopDir();
    const QString desktop2 = XdgDir::readDesktopDir();

    QCOMPARE(desktop1, expectedPath);
    QCOMPARE(desktop1, desktop2);
    QVERIFY(desktop1.startsWith(home_));
    QVERIFY(!desktop1.isEmpty());
}

void TestXdgDirFunctionality::testXdgDirPathValidation() {
    const QString expectedPath = home_ + QStringLiteral("/validated-desktop");
    XdgDir::setDesktopDir(expectedPath);

    const QString desktopDir = XdgDir::readDesktopDir();

    // The path should not contain double slashes (except at the beginning)
    QVERIFY(!desktopDir.contains(QStringLiteral("//")));

    // Should be an absolute path
    QVERIFY(QDir::isAbsolutePath(desktopDir));

    // Should not be a relative path
    QVERIFY(!desktopDir.startsWith(QStringLiteral(".")));
    QVERIFY(!desktopDir.startsWith(QStringLiteral("..")));
}

QTEST_MAIN(TestXdgDirFunctionality)
#include "test_xdgdir_functionality.moc"
