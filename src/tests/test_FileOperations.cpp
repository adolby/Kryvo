#include "FileOperations.hpp"
#include "catch.hpp"
#include <QFileInfo>
#include <QFile>
#include <QString>

SCENARIO("File compare returns success for identical files", "[compareSame]") {
  GIVEN("Two identical PNG files") {
    const QString& fileName1 = QStringLiteral("file1.png");
    const QString& fileName2 = QStringLiteral("file2.png");

    const QFileInfo fileInfo1(fileName1);
    const QFileInfo fileInfo2(fileName2);

    const QString& msgTemplate = QStringLiteral("Test file %1 is missing.");

    if (!fileInfo1.exists()) {
      FAIL(msgTemplate.arg(fileName1).toStdString());
    }

    if (!fileInfo2.exists()) {
      FAIL(msgTemplate.arg(fileName2).toStdString());
    }

    WHEN("Files are equivalent") {
      const bool equivalentTest = FileOperations::filesEqual(fileName1,
                                                             fileName2);
      THEN("Returns true") {
        REQUIRE(equivalentTest);
      }
    }
  }
}

SCENARIO("File compare returns success for different files",
         "[compareDifferent]") {
  GIVEN("Two different PNG files") {
    const QString& fileName1 = QStringLiteral("file1.png");
    const QString& fileName2 = QStringLiteral("file3.png");

    const QFileInfo fileInfo1(fileName1);
    const QFileInfo fileInfo2(fileName2);

    const QString& msgTemplate = QStringLiteral("Test file %1 is missing. ");

    if (!fileInfo1.exists()) {
      FAIL(msgTemplate.arg(fileName1).toStdString());
    }

    if (!fileInfo2.exists()) {
      FAIL(msgTemplate.arg(fileName2).toStdString());
    }

    WHEN("Files aren't equivalent") {
      const bool equivalentTest = FileOperations::filesEqual(fileName1,
                                                             fileName2);

      THEN("Returns false") {
        REQUIRE_FALSE(equivalentTest);
      }
    }
  }
}
