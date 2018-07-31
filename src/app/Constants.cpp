#include "Constants.hpp"

const QString Kryvo::Constants::kDot = QStringLiteral(".");
const QString Kryvo::Constants::kExtension = QStringLiteral("enc");
const QString Kryvo::Constants::kArchiveExtension = QStringLiteral("7z");

const QStringList Kryvo::Constants::messages {
  QObject::tr("File %1 encrypted."), // 0
  QObject::tr("File %1 decrypted."), // 1
  QObject::tr("Encryption stopped. File %1 not fully encrypted."), // 2
  QObject::tr("Decryption stopped. File %1 not fully decrypted."), // 3
  QObject::tr("Error: Can't read file %1."), // 4
  QObject::tr("Error: Can't decrypt file %1. Wrong password entered or the "
              "file has been corrupted."), // 5
  QObject::tr("Error: Can't decrypt file %1. Is it an encrypted file?"), // 6
  QObject::tr("Error: Can't encrypt file %1. Check that this file exists and "
              "that you have permission to access it and try again."), // 7
  QObject::tr("Error: Can't compress %1. Please contact "
              "andrewdolby@gmail.com"), // 8
  QObject::tr("Error: Can't extract %1. Please contact "
              "andrewdolby@gmail.com"), // 9
  QObject::tr("Unknown error: Please contact andrewdolby@gmail.com.") // 10
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
  const QFileInfo firstSuffixFileInfo{filePath};
  QString newFilePath = filePath;

  if (QStringLiteral("7z") == firstSuffixFileInfo.suffix()) {
    newFilePath = firstSuffixFileInfo.absolutePath() % QDir::separator() %
                  firstSuffixFileInfo.completeBaseName();
  }

  const QFileInfo secondSuffixFileInfo(newFilePath);

  if (secondSuffixFileInfo.suffix() == extension) {
    newFilePath = secondSuffixFileInfo.absolutePath() % QDir::separator() %
                  secondSuffixFileInfo.completeBaseName();
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
  const QFileInfo inputFile{filePath};
  QString uniqueFilePath = filePath;

  bool foundUniqueFilePath = false;
  auto i = 0;

  while (!foundUniqueFilePath && i < 100000) {
    const QFileInfo uniqueFile{uniqueFilePath};

    if (uniqueFile.exists() && uniqueFile.isFile()) {
      // Write number of copies before file extension
      uniqueFilePath = QString(inputFile.absolutePath() % QDir::separator() %
                               inputFile.baseName() %
                               QStringLiteral(" (%1)").arg(i + 2));

      const QString suffix = inputFile.completeSuffix();
      if (!suffix.isEmpty()) {
        // Add the file extension if there is one
        uniqueFilePath += QString(Constants::kDot % suffix);
      }

      ++i;
    }
    else {
      foundUniqueFilePath = true;
    }
  }

  return uniqueFilePath;
}
