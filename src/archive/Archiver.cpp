#include "src/archive/Archiver.hpp"
#include "quazipfile.h"
#include "quazipfileinfo.h"
#include <QDir>
#include <QFile>
#include <QStringBuilder>

#include <QDebug>

qint64 copyData(QIODevice& inFile, QIODevice& outFile) {
  char buffer[4096];
  const qint64 readSize = inFile.read(buffer, 4096);

  if (readSize <= 0) {
    return -1;
  }

  const qint64 writeSize = outFile.write(buffer, readSize);

  if (writeSize != readSize) {
    return -1;
  }

  return writeSize;
}

bool removeFiles(const QStringList& fileList) {
  bool ret = true;

  for (const QString& filePath : fileList) {
    ret = ret && QFile::remove(filePath);
  }

  return ret;
}

Kryvos::Archiver::Archiver(QObject* parent)
  : QObject{parent} {
}

bool Kryvos::Archiver::compressFiles(const QString& fileCompressed,
                                     const QStringList& encryptedFiles,
                                     const QStringList& originalFiles) {
  QuaZip zip{fileCompressed};

  QDir{}.mkpath(QFileInfo(fileCompressed).absolutePath());

  const bool createdZip = zip.open(QuaZip::mdCreate);

  if (!createdZip) {
    QFile::remove(fileCompressed);
    return false;
  }

  qint64 compressedSize = 0;

  for (auto i = 0; i < encryptedFiles.length(); ++i) {
    const QString file = encryptedFiles.at(i);
    const QString originalFile = originalFiles.at(i);

    QFileInfo info{file};

    compressedSize = compressFile(&zip, file, info.fileName(), originalFile);

    if (!info.exists() || (compressedSize <= 0)) {
      QFile::remove(fileCompressed);
      return false;
    }
  }

  zip.close();

  if (zip.getZipError() != 0) {
    QFile::remove(fileCompressed);
    return false;
  }

  return true;
}

QStringList Kryvos::Archiver::extractDir(const QString& fileCompressed,
                                         const QString& dir) {
  QuaZip zip{fileCompressed};

  return extractDir(zip, dir);
}

qint64 Kryvos::Archiver::copyFileData(QIODevice& inFile,
                                      QIODevice& outFile,
                                      const QString& filePath,
                                      const qint64 fileSize,
                                      const QString& task) {
  qint64 processedSize = 0;

  while (!inFile.atEnd()) {
    const qint64 writeSize = copyData(inFile, outFile);

    if (-1 == writeSize) {
      processedSize = -1;
      break;
    }

    processedSize += writeSize;

    const auto nextFraction = static_cast<double>(processedSize) /
                              static_cast<double>(fileSize);
    const auto nextPercent = static_cast<int>(nextFraction * 100);

    if (!task.isEmpty()) {
      if (nextPercent > 99) {
        if (tr("Compressing") == task) {
          emit progress(filePath, tr("Compressed"), nextPercent);
        }
        else if (tr("Extracting") == task) {
          emit progress(filePath, tr("Extracted"), nextPercent);
        }
      }
      else {
        emit progress(filePath, task, nextPercent);
      }
    }
  }

  return fileSize;
}

qint64 Kryvos::Archiver::compressFile(QuaZip* zip,
                                      const QString& fileName,
                                      const QString& fileDestination,
                                      const QString& originalFilePath) {
  if (!zip) {
    return -1;
  }

  if (zip->getMode() != QuaZip::mdCreate &&
      zip->getMode() != QuaZip::mdAppend &&
      zip->getMode() != QuaZip::mdAdd) {
    return -1;
  }

  QFile inFile{fileName};

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    return -1;
  }

  QuaZipFile outFile{zip};
  const bool outFileOpen =
    outFile.open(QIODevice::WriteOnly,
                 QuaZipNewInfo{fileDestination, inFile.fileName()});

  if (!outFileOpen) {
    return -1;
  }

  const QString task = tr("Compressing");
  const qint64 copiedDataSize = copyFileData(inFile, outFile, originalFilePath,
                                             inFile.size(), task);

  if (copiedDataSize <= 0 || (outFile.getZipError() != UNZ_OK)) {
    return -1;
  }

  outFile.close();

  if (outFile.getZipError() != UNZ_OK) {
    return -1;
  }

  inFile.close();

  return copiedDataSize;
}

QStringList Kryvos::Archiver::extractDir(QuaZip& zip, const QString& dir) {
  if (!zip.open(QuaZip::mdUnzip)) {
    return QStringList{};
  }

  QDir directory{dir};

  if (!zip.goToFirstFile()) {
    return QStringList{};
  }

  QStringList extracted{};
  qint64 extractedSize = 0;

  do {
    const QString& name = zip.getCurrentFileName();
    const QString& absFilePath = directory.absoluteFilePath(name);

    extractedSize = extractFile(&zip, QString{}, absFilePath);

    if (extractedSize <= 0) {
      removeFiles(extracted);
      return QStringList{};
    }

    extracted.append(absFilePath);
  } while (zip.goToNextFile());

  zip.close();

  if (zip.getZipError() != 0) {
    removeFiles(extracted);
    return QStringList{};
  }

  return extracted;
}

qint64 Kryvos::Archiver::extractFile(QuaZip* zip,
                                     const QString& fileName,
                                     const QString& fileDest) {
  if (!zip) {
    return -1;
  }

  if (zip->getMode() != QuaZip::mdUnzip) {
    return -1;
  }

  if (!fileName.isEmpty()) {
    zip->setCurrentFile(fileName);
  }

  QuaZipFile inFile{zip};

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen || inFile.getZipError() != UNZ_OK) {
    return -1;
  }

  QDir curDir{};

  if (fileDest.endsWith('/')) {
    const auto destPath = curDir.mkpath(fileDest);

    if (!destPath) {
      return -1;
    }
  }
  else {
    const auto destPath = curDir.mkpath(QFileInfo{fileDest}.absolutePath());

    if (!destPath) {
      return -1;
    }
  }

  QuaZipFileInfo64 info;
  if (!zip->getCurrentFileInfo(&info)) {
    return -1;
  }

  const auto srcPerm = info.getPermissions();

  if (fileDest.endsWith('/') && QFileInfo(fileDest).isDir()) {
    if (srcPerm != 0) {
      QFile{fileDest}.setPermissions(srcPerm);
    }

    return -1;
  }

  QFile outFile{};
  outFile.setFileName(fileDest);

  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    return -1;
  }

  const qint64 uncompressedSize = info.uncompressedSize;

  const QString task = tr("Extracting");
  const qint64 copiedDataSize = copyFileData(inFile, outFile, fileDest,
                                             uncompressedSize, task);

  if ((copiedDataSize <= 0) || inFile.getZipError() != UNZ_OK) {
    outFile.close();
    removeFiles(QStringList{fileDest});
    return -1;
  }

  outFile.close();
  inFile.close();

  if (inFile.getZipError() != UNZ_OK) {
    removeFiles(QStringList{fileDest});
    return -1;
  }

  if (srcPerm != 0) {
    outFile.setPermissions(srcPerm);
  }

  return copiedDataSize;
}
