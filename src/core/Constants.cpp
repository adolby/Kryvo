#include "Constants.hpp"
#include <QStandardPaths>
#include <QObject>
#include <QStringBuilder>

namespace Kryvo {

const QVersionNumber Constants::version({2, 0, 8});

const QString Constants::dot = QStringLiteral(".");
const QString Constants::encryptedFileExtension = QStringLiteral("enc");
const QString Constants::compressedFileExtension = QStringLiteral("gz");
const QString Constants::archiveFileExtension = QStringLiteral("tar");

const QStringList Constants::defaultPaths =
  QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
const QString Constants::documentsPath = QString(defaultPaths.first() %
                                                 QStringLiteral("/"));

const QStringList Constants::messages {
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
  QObject::tr("Error: No encryption providers found."), // 11
  QObject::tr("Encryption/decryption is already in progress. Please wait until "
              "the current operation finishes.") // 12
};

} // namespace Kryvo
