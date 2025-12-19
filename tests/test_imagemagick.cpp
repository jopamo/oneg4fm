
#include <QtTest>
#include <QImage>
#include <QFile>
#include <QTemporaryDir>
#include "../pcmanfm/imagemagick_support.h"

class TestImageMagick : public QObject {
    Q_OBJECT

   private slots:
    void initTestCase() {
        if (!ImageMagickSupport::isAvailable()) {
            QSKIP("ImageMagick not available, skipping tests");
        }
    }

    void testLoadJpeg() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString path = tempDir.filePath("test.jpg");

        // Create a simple red image
        QImage image(100, 100, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(path, "JPG"));

        ImageMagickBuffer buffer;
        QVERIFY(ImageMagickSupport::loadImageBuffer(path, buffer));

        QCOMPARE(buffer.width, 100);
        QCOMPARE(buffer.height, 100);
        QVERIFY(!buffer.pixels.empty());

        // Check a pixel (should be red)
        // Buffer is RGBA
        // Red is 255, 0, 0, 255

        const int stride = buffer.width * 4;
        const unsigned char* data = buffer.pixels.data();

        // Check pixel at 50, 50
        int x = 50;
        int y = 50;
        int offset = y * stride + x * 4;

        // Allow some tolerance due to JPEG compression
        int r = data[offset];
        int g = data[offset + 1];
        int b = data[offset + 2];
        int a = data[offset + 3];

        qDebug() << "Pixel at 50,50: R=" << r << " G=" << g << " B=" << b << " A=" << a;

        QVERIFY(r > 200);
        QVERIFY(g < 50);
        QVERIFY(b < 50);
        // Alpha should be 255 (opaque)
        QCOMPARE(a, 255);
    }
};

QTEST_MAIN(TestImageMagick)
#include "test_imagemagick.moc"
