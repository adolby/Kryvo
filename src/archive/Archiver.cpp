#include "Archiver.hpp"
#include "quazipfile.h"
#include "quazipfileinfo.h"
#include <QDir>
#include <QFile>

bool removeFiles(const QStringList& fileList) {
  bool ret = true;

  for (const auto& filePath : fileList) {
    ret = ret && QFile::remove(filePath);
  }

  return ret;
}

Archiver::Archiver(QObject* parent) :
  QObject{parent} {
}

bool Archiver::compressFiles(const QString& fileCompressed,
                             const QStringList& files) {
  QuaZip zip{fileCompressed};

  QDir{}.mkpath(QFileInfo(fileCompressed).absolutePath());

  const bool createdZip = zip.open(QuaZip::mdCreate);

  if (!createdZip) {
    QFile::remove(fileCompressed);
    return false;
  }

  QFileInfo info{};

  for (const auto& file : files) {
    info.setFile(file);

    const bool compressedFile = compressFile(&zip, file, info.fileName());

    if (!info.exists() || !compressedFile) {
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

QStringList Archiver::extractDir(const QString& fileCompressed,
                                 const QString& dir) {
  QuaZip zip{fileCompressed};

  return extractDir(zip, dir);
}

QString Archiver::extractFile(QIODevice* ioDevice, const QString& fileName,
                              const QString& fileDest) {
  QuaZip zip{ioDevice};

  return extractFile(zip, fileName, fileDest);
}

bool Archiver::copyData(QIODevice& inFile, QIODevice& outFile) {
  while (!inFile.atEnd()) {
    char buffer[4096];
    const qint64 readLength = inFile.read(buffer, 4096);

    if (readLength <= 0) {
      return false;
    }

    const qint64 writeLength = outFile.write(buffer, readLength);

    if (writeLength != readLength) {
      return false;
    }

    int percentProgress = 5;
    emit progress(percentProgress);
  }

  return true;
}

bool Archiver::compressFile(QuaZip* zip, const QString& fileName,
                            const QString& fileDestination) {
  if (!zip) {
    return false;
  }

  if (zip->getMode() != QuaZip::mdCreate &&
      zip->getMode() != QuaZip::mdAppend &&
      zip->getMode() != QuaZip::mdAdd) {
    return false;
  }

  QFile inFile{};
  inFile.setFileName(fileName);

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    return false;
  }

  QuaZipFile outFile{zip};
  const bool outFileOpen =
    outFile.open(QIODevice::WriteOnly,
                 QuaZipNewInfo{fileDestination, inFile.fileName()});

  if (!outFileOpen) {
    return false;
  }

  const bool copiedData = copyData(inFile, outFile);

  if (!copiedData || (outFile.getZipError() != UNZ_OK)) {
    return false;
  }

  outFile.close();

  if (outFile.getZipError() != UNZ_OK) {
    return false;
  }

  inFile.close();

  return true;
}

QStringList Archiver::extractDir(QuaZip& zip, const QString& dir) {
  if (!zip.open(QuaZip::mdUnzip)) {
    return QStringList{};
  }

  QDir directory{dir};

  QStringList extracted{};

  if (!zip.goToFirstFile()) {
    return QStringList{};
  }

  do {
    const QString& name = zip.getCurrentFileName();
    const QString& absFilePath = directory.absoluteFilePath(name);

    const bool extractFileSuccess = extractFile(&zip, "", absFilePath);

    if (!extractFileSuccess) {
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

QString Archiver::extractFile(QuaZip& zip, const QString& fileName,
                              const QString& fileDestination) {
  if (!zip.open(QuaZip::mdUnzip)) {
    return QString{};
  }

  const QString& fileDest = fileDestination.isEmpty() ? fileName :
                                                        fileDestination;

  const bool extractedFile = extractFile(&zip,fileName,fileDest);

  if (!extractedFile) {
    return QString{};
  }

  zip.close();

  if (zip.getZipError() != 0) {
    removeFiles(QStringList{fileDest});
    return QString{};
  }

  return QFileInfo{fileDest}.absoluteFilePath();
}

bool Archiver::extractFile(QuaZip* zip, const QString& fileName,
                           const QString& fileDest) {
  if (!zip) {
    return false;
  }

  if (zip->getMode() != QuaZip::mdUnzip) {
    return false;
  }

  if (!fileName.isEmpty()) {
    zip->setCurrentFile(fileName);
  }

  QuaZipFile inFile{zip};

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen || inFile.getZipError() != UNZ_OK) {
    return false;
  }

  QDir curDir{};

  if (fileDest.endsWith('/')) {
    if (!curDir.mkpath(fileDest)) {
      return false;
    }
  } else {
    if (!curDir.mkpath(QFileInfo(fileDest).absolutePath())) {
      return false;
    }
  }

  QuaZipFileInfo64 info{};
  if (!zip->getCurrentFileInfo(&info)) {
    return false;
  }

  const auto srcPerm = info.getPermissions();

  if (fileDest.endsWith('/') && QFileInfo(fileDest).isDir()) {
    if (srcPerm != 0) {
      QFile{fileDest}.setPermissions(srcPerm);
    }

    return true;
  }

  QFile outFile{};
  outFile.setFileName(fileDest);

  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    return false;
  }

  const bool copiedData = copyData(inFile, outFile);

  if (!copiedData || inFile.getZipError() != UNZ_OK) {
    outFile.close();
    removeFiles(QStringList{fileDest});
    return false;
  }

  outFile.close();
  inFile.close();

  if (inFile.getZipError() != UNZ_OK) {
    removeFiles(QStringList{fileDest});
    return false;
  }

  if (srcPerm != 0) {
    outFile.setPermissions(srcPerm);
  }

  return true;
}
