#ifndef KRYVOS_CRYPTOGRAPHY_CONSTANTS_H_
#define KRYVOS_CRYPTOGRAPHY_CONSTANTS_H_

#include <QFileInfo>
#include <QDir>
#include <QObject>
#include <QStringList>
#include <QString>
#include <QStringBuilder>

const auto kExtension = QStringLiteral("enc");

const QStringList messages
{
 QObject::tr("File %1 encrypted."), // 0
 QObject::tr("File %1 decrypted."), // 1
 QObject::tr("Encryption stopped. File %1 is incomplete."), // 2
 QObject::tr("Decryption stopped. File %1 is incomplete."), // 3
 QObject::tr("Error: Can't read file %1."), // 4
 QObject::tr("Error: Can't decrypt file %1. Wrong password entered or the file"
             "has been corrupted."), // 5
 QObject::tr("Error: Can't decrypt file %1. Is it an encrypted file?"), // 6
 QObject::tr("Error: Can't encrypt file %1. Check that this file exists and "
             "that you have permission to access it and try again."), // 7
 QObject::tr("Unknown error: Please contact andrewdolby@gmail.com.") // 8
};

/*!
 * \brief removeExtension Attempts to return the file path string input
 * without the last extension. It's used to extract an extension to determine
 * a decrypted file path.
 * \param filePath String containing the file path
 * \param extension String representing the extension to remove from the
 * file path
 * \return String representing a file path without an extension
 */
inline QString removeExtension(const QString& filePath,
                               const QString& extension)
{
  QFileInfo fileInfo{filePath};
  QString newFilePath = filePath;

  if (fileInfo.suffix() == extension)
  {
    newFilePath = fileInfo.absolutePath() % QDir::separator() %
                  fileInfo.completeBaseName();
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
inline QString uniqueFilePath(const QString& filePath)
{
  QFileInfo originalFile{filePath};
  QString uniqueFilePath = filePath;

  auto foundUniqueFilePath = false;
  auto i = 0;

  while (!foundUniqueFilePath && i < 100000)
  {
    QFileInfo uniqueFile{uniqueFilePath};

    if (uniqueFile.exists() && uniqueFile.isFile())
    { // Write number of copies before file extension
      uniqueFilePath = originalFile.absolutePath() % QDir::separator() %
                       originalFile.baseName() % QString{" (%1)"}.arg(i + 2);

      if (!originalFile.completeSuffix().isEmpty())
      { // Add the file extension if there is one
        uniqueFilePath += QStringLiteral(".") % originalFile.completeSuffix();
      }

      ++i;
    }
    else
    {
      foundUniqueFilePath = true;
    }
  }

  return uniqueFilePath;
}

#endif // KRYVOS_CRYPTOGRAPHY_CONSTANTS_H_
