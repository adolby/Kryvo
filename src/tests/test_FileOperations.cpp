#include "FileOperations.h"
#include "catch.hpp"
#include <QFile>
#include <QString>


SCENARIO("File compare returns success for identical files", "[compareSame]") {
  GIVEN("Two identical PNG files") {
    // Test data
    const QString& fileName1 = QStringLiteral("file1.png");
    const QString& fileName2 = QStringLiteral("file2.png");

    const QFile file1(fileName1);
    const QFile file2(fileName2);

    if (!file1.exists() || !file2.exists()) {
      const QString& msg = QStringLiteral("Test file %1 is missing. ");

      QString message;

      if (!file1.exists()) {
        message += msg.arg(fileName1);
      }

      if (!file2.exists()) {
        message += msg.arg(fileName2);
      }

      const char* messages = reinterpret_cast<const char*>(message.constData());
    }

    const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

    REQUIRE(equivalentTest);
  }
}

SCENARIO("File compare returns success for different files",
         "[compareDifferent]") {
  GIVEN("Two different PNG files") {
    // Test data
    const QString& fileName1 = QStringLiteral("file1.png");
    const QString& fileName2 = QStringLiteral("file3.png");

    const QFile file1(fileName1);
    const QFile file2(fileName2);

    if (!file1.exists() || !file2.exists()) {
      const QString& msg = QStringLiteral("Test file %1 is missing. ");

      QString message;

      if (!file1.exists()) {
        message += msg.arg(fileName1);
      }

      if (!file2.exists()) {
        message += msg.arg(fileName2);
      }

      const char* messages = reinterpret_cast<const char*>(message.constData());
    }

    const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

    REQUIRE(!equivalentTest);
  }
}
