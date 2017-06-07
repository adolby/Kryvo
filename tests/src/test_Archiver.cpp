#include "test_Archiver.hpp"
#include "FileOperations.h"
#include "src/archive/Archiver.hpp"
#include <QTest>
#include <QFile>
#include <QThread>
#include <QString>

void TestArchiver::testComparatorSameFile() {
  // Test data
  const QString fileName1 = QStringLiteral("file1.png");
  const QString fileName2 = QStringLiteral("file2.png");

  QFile file1{fileName1};
  QFile file2{fileName2};
  if (!file1.exists() || !file2.exists()) {
    QString message;
    const QString msg = QStringLiteral("Test file %1 is missing. ");

    if (!file1.exists()) {
      message += msg.arg(fileName1);
    }

    if (!file2.exists()) {
      message += msg.arg(fileName2);
    }

    QSKIP(message.toStdString().c_str());
  }

  const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

  QVERIFY(equivalentTest);
}

void TestArchiver::testComparatorDifferentFile() {
  // Test data
  const QString fileName1 = QStringLiteral("file1.png");
  const QString fileName2 = QStringLiteral("file3.png");

  QFile file1{fileName1};
  QFile file2{fileName2};
  if (!file1.exists() || !file2.exists()) {
    QString message;
    const QString msg = QStringLiteral("Test file %1 is missing. ");

    if (!file1.exists()) {
      message += msg.arg(fileName1);
    }

    if (!file2.exists()) {
      message += msg.arg(fileName2);
    }

    QSKIP(message.toStdString().c_str());
  }

  const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

  QVERIFY(!equivalentTest);
}

QTEST_GUILESS_MAIN(TestArchiver)
