#include "Constants.hpp"
#include <QStandardPaths>
#include <QStringBuilder>

const QVersionNumber Kryvo::Constants::kVersion({1, 0, 11});

const QString Kryvo::Constants::kDot = QStringLiteral(".");
const QString Kryvo::Constants::kEncryptedFileExtension = QStringLiteral("enc");
const QString Kryvo::Constants::kCompressedFileExtension = QStringLiteral("gz");
const QString Kryvo::Constants::kArchiveFileExtension = QStringLiteral("tar");

const QStringList Kryvo::Constants::kDefaultPaths =
  QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
const QString Kryvo::Constants::kDocumentsPath = QString(kDefaultPaths.first() %
                                                         QStringLiteral("/"));

const QStringList Kryvo::Constants::kMessages {
  QObject::tr("Unknown error: Please contact andrewdolby@gmail.com."), // 0
  QObject::tr("File %1 encrypted."), // 1
  QObject::tr("File %1 decrypted."), // 2
  QObject::tr("Encryption stopped. File %1 not fully encrypted."), // 3
  QObject::tr("Decryption stopped. File %1 not fully decrypted."), // 4
  QObject::tr("Error: Can't read file %1."), // 5
  QObject::tr("Error: Can't decrypt file %1. Wrong password entered or the "
              "file has been corrupted."), // 6
  QObject::tr("Error: Can't decrypt file %1. Is it an encrypted file?"), // 7
  QObject::tr("Error: Can't encrypt file %1. Check that this file exists and "
              "that you have permission to access it and try again."), // 8
  QObject::tr("Compression stopped. File %1 not fully compressed."), // 9
  QObject::tr("Decompression stopped. File %1 not fully decompressed."), // 10
  QObject::tr("Error: No encryption providers found.") // 11
};

/*!
 * \brief removeExtension Attempts to return the file path string input
 * without the last extension. It's used to extract an extension to determine
 * a decrypted file path.
 * \param filePath String containing the file path
 * \param extension String representing the extension to remove from the
 * file path
 * \return String containing a file path without an extension
 */
QString Kryvo::Constants::removeExtension(const QString& filePath,
                                          const QString& extension) {
  const QFileInfo suffixFileInfo(filePath);

  QString newFilePath = filePath;

  if (suffixFileInfo.suffix() == extension) {
    newFilePath = suffixFileInfo.absolutePath() % QStringLiteral("/") %
                  suffixFileInfo.completeBaseName();
  }

  return newFilePath;
}

/*!
 * \brief uniqueFilePath Returns a unique file path from the input file path
 * by appending an increasing number, if necessary.
 * \param filePath String representing the file path that will be tested
 * for uniqueness
 * \return String representing a unique file path created from the input file
 * path
 */
QString Kryvo::Constants::uniqueFilePath(const QString& filePath) {
  const QFileInfo inputFile(filePath);
  QString uniqueFilePath = filePath;

  bool foundUniqueFilePath = false;
  auto i = 0;

  while (!foundUniqueFilePath && i < 100000) {
    const QFileInfo uniqueFile(uniqueFilePath);

    if (uniqueFile.exists() && uniqueFile.isFile()) {
      // Write number of copies before file extension
      uniqueFilePath = QString(inputFile.absolutePath() % QStringLiteral("/") %
                               inputFile.baseName() %
                               QStringLiteral(" (%1)").arg(i + 2));

      const QString& suffix = inputFile.completeSuffix();
      if (!suffix.isEmpty()) {
        // Add the file extension if there is one
        uniqueFilePath += QString(Constants::kDot % suffix);
      }

      ++i;
    } else {
      foundUniqueFilePath = true;
    }
  }

  return uniqueFilePath;
}
