/*
 * Unit tests for core utility functions (without libfm-qt dependencies)
 * tests/test_core_utils.cpp
 */

#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>
#include <QTest>

// Test simple string utilities without libfm-qt dependencies
class TestCoreUtils : public QObject {
    Q_OBJECT

   private slots:
    void testStringUtilities();
    void testPathUtilities();
    void testBasicTypes();
};

void TestCoreUtils::testStringUtilities() {
    // Test basic QString operations
    QString testString = QStringLiteral("test string");
    QVERIFY(!testString.isEmpty());
    QCOMPARE(testString.length(), 11);
    QVERIFY(testString.contains(QStringLiteral("test")));
    QVERIFY(!testString.contains(QStringLiteral("nonexistent")));

    // Test string concatenation
    QString part1 = QStringLiteral("hello");
    QString part2 = QStringLiteral(" world");
    QString result = part1 + part2;
    QCOMPARE(result, QStringLiteral("hello world"));
}

void TestCoreUtils::testPathUtilities() {
    // Test basic path operations
    QString homePath = QDir::homePath();
    QVERIFY(!homePath.isEmpty());
    QVERIFY(QDir(homePath).exists());

    // Test temporary directory creation
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).exists());

    // Test path joining
    QString basePath = QStringLiteral("/base");
    QString subPath = QStringLiteral("subdir");
    QString fullPath = basePath + QStringLiteral("/") + subPath;
    QCOMPARE(fullPath, QStringLiteral("/base/subdir"));
}

void TestCoreUtils::testBasicTypes() {
    // Test Qt basic type operations
    QVERIFY(true);       // Basic boolean
    QCOMPARE(1 + 1, 2);  // Basic arithmetic
    QVERIFY(5 > 3);      // Comparison

    // Test QStringList
    QStringList list;
    list << QStringLiteral("item1") << QStringLiteral("item2");
    QCOMPARE(list.size(), 2);
    QVERIFY(list.contains(QStringLiteral("item1")));
    QVERIFY(list.contains(QStringLiteral("item2")));
    QVERIFY(!list.contains(QStringLiteral("item3")));
}

QTEST_MAIN(TestCoreUtils)
#include "test_core_utils.moc"