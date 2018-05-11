#include "archive/Archiver.hpp"
#include <QDir>
#include <QFile>
#include <QStringBuilder>

#include <QDebug>

Kryvo::Archiver::Archiver(QObject* parent)
  : QObject{parent} {
}

void Kryvo::Archiver::compress(const QStringList& inFilePaths,
                               const QString& outFilePath) {
  if (inFilePaths.isEmpty()) {
    return;
  }

  const QString filePath = outFilePath.isEmpty() ?
                           inFilePaths.first() :
                           outFilePath;

  for (const QString& inFilePath : inFilePaths) {

  }

  emit compressedFilePath(filePath);
}

void Kryvo::Archiver::extract(const QString& inFilePath,
                              const QString& outFilePath) {
  if (inFilePath.isEmpty()) {
    return;
  }

  const QFileInfo inFileInfo{inFilePath};
  const QString inPath = inFileInfo.absolutePath();

  const QString path = outFilePath.isEmpty() ? inPath : outFilePath;

  emit extractedFilePath(path);
}
