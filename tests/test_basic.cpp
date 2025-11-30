/*
 * Basic unit test to verify test infrastructure
 * tests/test_basic.cpp
 */

#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QTest>

class TestBasic : public QObject {
    Q_OBJECT

   private slots:
    void testBasicAssertions();
    void testStringOperations();
    void testArithmetic();
};

void TestBasic::testBasicAssertions() {
    // Basic boolean assertions
    QVERIFY(true);
    QVERIFY(!false);
    QVERIFY(1 == 1);
    QVERIFY(2 != 3);
}

void TestBasic::testStringOperations() {
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

void TestBasic::testArithmetic() {
    // Test basic arithmetic operations
    QCOMPARE(1 + 1, 2);
    QCOMPARE(5 - 3, 2);
    QCOMPARE(2 * 3, 6);
    QCOMPARE(6 / 2, 3);
    QVERIFY(5 > 3);
    QVERIFY(3 < 5);
    QVERIFY(4 >= 4);
    QVERIFY(4 <= 4);
}

// Use QTEST_APPLESS_MAIN for tests that don't need QApplication
QTEST_APPLESS_MAIN(TestBasic)
#include "test_basic.moc"