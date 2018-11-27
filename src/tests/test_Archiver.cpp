#include "FileOperations.hpp"
#include "archive/Archiver.hpp"
#include "catch.hpp"
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QString>


/*!
 * testCompressDecompress Tests the compression/decompression process with a
 * text file as input.
 */
SCENARIO("Test compression and decompression on a text file",
         "[testCompressDecompressText]") {
  GIVEN("A text file") {
    Kryvo::Archiver archiver;

    const QStringList& inputFileNames = {QStringLiteral("test1.txt")};
    const QStringList& compressedFileNames = {QStringLiteral("test1.txt.gz")};
    const QStringList& decompressedFileNames =
      {QStringLiteral("test1 (2).txt")};

    const QFileInfo inputFileInfo(inputFileNames.at(0));

    const QString& msgTemplate = QStringLiteral("Test file %1 is missing.");

    if (!inputFileInfo.exists()) {
      FAIL(msgTemplate.arg(inputFileNames.at(0)).toStdString());
    }

    WHEN("Compressing and decompressing") {
      const bool compressSuccess = archiver.compressFiles(inputFileNames);

      REQUIRE(compressSuccess);

      const bool decompressSuccess =
        archiver.decompressFiles(compressedFileNames);

      REQUIRE(decompressSuccess);

      // Compare initial file with decompressed file
      const bool equivalentTest =
        FileOperations::filesEqual(inputFileNames.at(0),
                                   decompressedFileNames.at(0));

      // Clean up test files
      QFile compressedFile(compressedFileNames.at(0));

      if (compressedFile.exists()) {
        compressedFile.remove();
      }

      QFile decompressedFile(decompressedFileNames.at(0));

      if (decompressedFile.exists()) {
        decompressedFile.remove();
      }

      THEN("Decompressed file matches original file") {
        REQUIRE(equivalentTest);
      }
    }
  }
}
