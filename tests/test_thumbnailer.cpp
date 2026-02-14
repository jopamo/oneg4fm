#include <QtTest>
#include <QString>
#include <gio/gio.h>

#include <libfm-qt6/core/thumbnailer.h>

class TestThumbnailer : public QObject {
    Q_OBJECT

   private Q_SLOTS:
    void testOutputPathPlaceholderIsSingleArg() {
        GKeyFile* keyFile = g_key_file_new();
        QVERIFY(keyFile != nullptr);

        g_key_file_set_string(keyFile, "Thumbnailer Entry", "Exec", "thumbgen %u %o %s");

        Fm::Thumbnailer thumbnailer("test.thumbnailer", keyFile);
        auto command = thumbnailer.commandForUri("file:///tmp/input%20image.png", "/tmp/thumb cache/out file.png", 128);
        QVERIFY(command != nullptr);

        int argc = 0;
        char** argv = nullptr;
        GError* parseError = nullptr;
        QVERIFY(g_shell_parse_argv(command.get(), &argc, &argv, &parseError));
        QVERIFY(parseError == nullptr);

        QCOMPARE(argc, 4);
        QCOMPARE(QString::fromUtf8(argv[0]), QString("thumbgen"));
        QCOMPARE(QString::fromUtf8(argv[1]), QString("file:///tmp/input%20image.png"));
        QCOMPARE(QString::fromUtf8(argv[2]), QString("/tmp/thumb cache/out file.png"));
        QCOMPARE(QString::fromUtf8(argv[3]), QString("128"));

        g_strfreev(argv);
        g_clear_error(&parseError);
        g_key_file_free(keyFile);
    }
};

QTEST_MAIN(TestThumbnailer)
#include "test_thumbnailer.moc"
