#ifndef KRYVO_FILEUTILITY_H_
#define KRYVO_FILEUTILITY_H_

#include "Constants.hpp"
#include <QDir>
#include <QSaveFile>
#include <QFileInfo>
#include <QFile>
#include <QString>
#include <QStringBuilder>
#include <QByteArray>
#include <QHash>

namespace Kryvo {

/*!
 * \brief removeExtension Attempts to return the file path string input
 * without the last extension. It's used to extract an extension to determine
 * a decrypted file path.
 * \param filePath String containing the file path
 * \param extension String representing the extension to remove from the
 * file path
 * \return String containing a file path without an extension
 */
inline QString removeExtension(const QString& filePath,
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
inline QString uniqueFilePath(const QString& filePath) {
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

inline QHash<QByteArray, QByteArray> readHeader(QFile* file) {
  QHash<QByteArray, QByteArray> headerData;

  // Read metadata from file

  // Read line but skip \n
  auto readLine = [](QFile* file) {
    if (file) {
      QByteArray line = file->readLine();
      return line.replace(QByteArrayLiteral("\n"), QByteArrayLiteral(""));
    }

    return QByteArray();
  };

  if (file->atEnd()) {
    return headerData;
  }

  // Read start header line
  const QByteArray& headerString = readLine(file);

  while (!file->atEnd()) {
    const QByteArray& line = readLine(file);

    if (QByteArrayLiteral("------------------------------------") == line) {
      // Read end header line
      break;
    }

    const QList<QByteArray>& keyValuePair = line.split(':');

    if (keyValuePair.size() < 2) {
      break;
    }

    const QByteArray& key = keyValuePair.at(0);
    const QByteArray& value = keyValuePair.at(1).trimmed();

    headerData.insert(key, value);
  }

  return headerData;
}

inline void writeHeader(QSaveFile* file,
                        const QHash<QByteArray, QByteArray>& headerData) {
  file->write(QByteArrayLiteral("---------- Encrypted File ----------"));
  file->write(QByteArrayLiteral("\n"));

  auto end = headerData.cend();
  for (auto it = headerData.cbegin(); it != end; ++it) {
    const QByteArray& key = it.key();
    const QByteArray& val = it.value();

    QByteArray headerEntry = key;

    if (!val.isEmpty()) {
      headerEntry = headerEntry + QByteArrayLiteral(": ") + val;
    }

    file->write(headerEntry);
    file->write(QByteArrayLiteral("\n"));
  }

  file->write(QByteArrayLiteral("------------------------------------"));
  file->write(QByteArrayLiteral("\n"));
}

} // namespace Kryvo

#endif // KRYVO_FILEUTILITY_H_
