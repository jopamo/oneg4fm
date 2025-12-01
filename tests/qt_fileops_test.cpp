/*
 * Tests for Qt file operations adapter using the POSIX core
 * tests/qt_fileops_test.cpp
 */

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "../src/backends/qt/qt_fileops.h"

#include <QFile>
#include <QFileInfo>

using namespace PCManFM;

class QtFileOpsTest : public QObject {
    Q_OBJECT

   private slots:
    void copyFile();
    void moveFile();
    void deleteFile();
};

static QString writeTempFile(const QTemporaryDir& dir, const QString& name, const QByteArray& data) {
    const QString path = dir.path() + QLatin1Char('/') + name;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(data);
        f.close();
    }
    return path;
}

void QtFileOpsTest::copyFile() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString src = writeTempFile(dir, QStringLiteral("src.txt"), "copy-data");
    const QString dstDir = dir.path() + QLatin1String("/dst");
    QVERIFY(QDir().mkpath(dstDir));

    QtFileOps ops;
    QSignalSpy finishedSpy(&ops, &QtFileOps::finished);

    FileOpRequest req;
    req.type = FileOpType::Copy;
    req.sources = QStringList{src};
    req.destination = dstDir;
    req.followSymlinks = false;
    req.overwriteExisting = false;

    ops.start(req);

    QTRY_VERIFY_WITH_TIMEOUT(finishedSpy.count() > 0, 2000);
    const QList<QVariant> args = finishedSpy.takeFirst();
    QVERIFY(args.at(0).toBool());
    const QString copied = dstDir + QLatin1String("/src.txt");
    QVERIFY(QFileInfo::exists(copied));
    QCOMPARE(QFile(copied).size(), QFile(src).size());
}

void QtFileOpsTest::moveFile() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString src = writeTempFile(dir, QStringLiteral("move.txt"), "move-data");
    const QString dstDir = dir.path() + QLatin1String("/dst2");
    QVERIFY(QDir().mkpath(dstDir));

    QtFileOps ops;
    QSignalSpy finishedSpy(&ops, &QtFileOps::finished);

    FileOpRequest req;
    req.type = FileOpType::Move;
    req.sources = QStringList{src};
    req.destination = dstDir;
    req.followSymlinks = false;
    req.overwriteExisting = false;

    ops.start(req);

    QTRY_VERIFY_WITH_TIMEOUT(finishedSpy.count() > 0, 2000);
    const QList<QVariant> args = finishedSpy.takeFirst();
    QVERIFY(args.at(0).toBool());
    const QString moved = dstDir + QLatin1String("/move.txt");
    QVERIFY(QFileInfo::exists(moved));
    QVERIFY(!QFileInfo::exists(src));
}

void QtFileOpsTest::deleteFile() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString src = writeTempFile(dir, QStringLiteral("delete.txt"), "delete-data");
    QVERIFY(QFileInfo::exists(src));

    QtFileOps ops;
    QSignalSpy finishedSpy(&ops, &QtFileOps::finished);

    FileOpRequest req;
    req.type = FileOpType::Delete;
    req.sources = QStringList{src};
    req.destination.clear();
    req.followSymlinks = false;
    req.overwriteExisting = false;

    ops.start(req);

    QTRY_VERIFY_WITH_TIMEOUT(finishedSpy.count() > 0, 2000);
    const QList<QVariant> args = finishedSpy.takeFirst();
    QVERIFY(args.at(0).toBool());
    QVERIFY(!QFileInfo::exists(src));
}

QTEST_MAIN(QtFileOpsTest)
#include "qt_fileops_test.moc"
