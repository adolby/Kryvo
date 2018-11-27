#include "FileOperations.hpp"
#include <QFile>
#include <QByteArray>

bool FileOperations::filesEqual(const QString& filePath1,
                                const QString& filePath2) {
  QFile file1(filePath1);
  QFile file2(filePath2);

  if (!file1.exists() || !file2.exists()) {
    return false;
  }

  if (file1.size() != file2.size()) {
    return false;
  }

  const bool file1Open = file1.open(QFile::ReadOnly);

  if (!file1Open) {
    return false;
  }

  const bool file2Open = file2.open(QFile::ReadOnly);

  if (!file2Open) {
    return false;
  }

  while (!file1.atEnd()) {
    QByteArray file1Buffer;
    file1Buffer.resize(4096);
    file1Buffer.fill(0);

    const qint64 file1BytesRead = file1.read(file1Buffer.data(), 4096);

    if (-1 == file1BytesRead) {
      return false;
    }

    QByteArray file2Buffer;
    file2Buffer.resize(4096);
    file2Buffer.fill(0);

    const qint64 file2BytesRead = file2.read(file2Buffer.data(), 4096);

    if (-1 == file2BytesRead) {
      return false;
    }

    if (file1Buffer != file2Buffer) {
      return false;
    }
  }

  return true;
}
