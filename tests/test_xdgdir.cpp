/*
 * Unit tests for XdgDir utility functions
 * tests/test_xdgdir.cpp
 */

#include <QFile>
#include <QObject>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

// Include the xdgdir header to test the utility functions
#include "xdgdir.h"

class TestXdgDir : public QObject {
    Q_OBJECT

   private slots:
    void initTestCase();
    void testReadDesktopDir();
    void testSetDesktopDir();
    void testReadUserDirsFile();

   private:
    QTemporaryDir configDir_;
};

void TestXdgDir::initTestCase() {
    QVERIFY(configDir_.isValid());
    qputenv("XDG_CONFIG_HOME", configDir_.path().toUtf8());
    QFile::remove(configDir_.filePath(QStringLiteral("user-dirs.dirs")));
}

void TestXdgDir::testReadDesktopDir() {
    const QString expectedDefault =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QStringLiteral("/Desktop");

    QCOMPARE(XdgDir::readDesktopDir(), expectedDefault);
}

void TestXdgDir::testSetDesktopDir() {
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString testPath = home + QStringLiteral("/test-desktop");

    XdgDir::setDesktopDir(testPath);

    const QString userDirsPath = configDir_.filePath(QStringLiteral("user-dirs.dirs"));
    QVERIFY(QFile::exists(userDirsPath));

    QFile userDirs(userDirsPath);
    QVERIFY(userDirs.open(QIODevice::ReadOnly));
    const QString contents = QString::fromUtf8(userDirs.readAll());
    QVERIFY(contents.contains(QStringLiteral("XDG_DESKTOP_DIR=\"$HOME/test-desktop\"")));

    QCOMPARE(XdgDir::readDesktopDir(), testPath);
}

void TestXdgDir::testReadUserDirsFile() {
    const QString userDirsPath = configDir_.filePath(QStringLiteral("user-dirs.dirs"));
    QFile userDirs(userDirsPath);
    QVERIFY(userDirs.open(QIODevice::WriteOnly | QIODevice::Truncate));
    const QByteArray data = QByteArrayLiteral("XDG_DESKTOP_DIR=\"$HOME/custom-desktop\"\n");
    QVERIFY(userDirs.write(data) == data.size());
    userDirs.close();

    const QString desktopDir = XdgDir::readDesktopDir();
    const QString expectedDesktop =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QStringLiteral("/custom-desktop");

    QCOMPARE(desktopDir, expectedDesktop);
}

QTEST_MAIN(TestXdgDir)
#include "test_xdgdir.moc"
