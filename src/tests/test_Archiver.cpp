#include "SchedulerState.hpp"
#include "archive/Archiver.hpp"
#include "FileOperations.hpp"
#include "doctest.h"
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QString>


/*!
 * testCompressDecompress Tests the compression/decompression process with a
 * text file as input.
 */
SCENARIO("testCompressDecompressText") {
  INFO("Test compression and decompression on a text file");

  GIVEN("A text file") {
    const QString inputFilePath = QStringLiteral("test1.txt");
    const QString compressedFilePath = QStringLiteral("test1.txt.gz");
    const QString decompressedFilePath = QStringLiteral("test1 (2).txt");

    const QString filePathMessage =
      QStringLiteral("File name: %1").arg(inputFilePath);

    INFO(filePathMessage.toStdString());

    Kryvo::SchedulerState state;
    Kryvo::Archiver archiver(&state);

    const QFileInfo inputFileInfo(inputFilePath);

    const QString msgTemplate = QStringLiteral("Test file %1 is missing.");

    if (!inputFileInfo.exists()) {
      FAIL(msgTemplate.arg(inputFilePath).toStdString());
    }

    WHEN("Compressing and decompressing file") {
      const std::size_t id = 0;

      Kryvo::CompressFileConfig compressConfig;
      compressConfig.inputFileInfo = QFileInfo(inputFilePath);
      compressConfig.outputFileInfo = QFileInfo(compressedFilePath);

      const bool compressed = archiver.compressFile(id, compressConfig);

      Kryvo::DecompressFileConfig decompressConfig;
      decompressConfig.inputFileInfo = QFileInfo(compressedFilePath);
      decompressConfig.outputFileInfo = QFileInfo(decompressedFilePath);

      const bool decompressed = archiver.decompressFile(id, decompressConfig);

      // Compare initial file with decompressed file
      const bool equivalentTest =
        FileOperations::filesEqual(inputFilePath, decompressedFilePath);

      // Clean up test files
      QFile compressedFile(compressedFilePath);

      if (compressedFile.exists()) {
        compressedFile.remove();
      }

      QFile decompressedFile(decompressedFilePath);

      if (decompressedFile.exists()) {
        decompressedFile.remove();
      }

      THEN("Decompressed file matches original file") {
        REQUIRE(compressed);
        REQUIRE(decompressed);
        REQUIRE(equivalentTest);
      }
    }
  }
}
