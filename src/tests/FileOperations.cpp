#include "FileOperations.h"
#include <QFile>
#include <QtGlobal>

bool FileOperations::filesEqual(const QString& filePath1,
                                const QString& filePath2) {
  bool equivalent = false;

  QFile file1(filePath1);
  QFile file2(filePath2);

  if (file1.exists() && file2.exists()) {
    file1.open(QFile::ReadOnly);
    file2.open(QFile::ReadOnly);

    while (!file1.atEnd()) {
      QByteArray file1Buffer;

      const qint64 file1BytesRead = file1.read(file1Buffer.data(), 16384);

      if (-1 == file1BytesRead) {
        break;
      }

      QByteArray file2Buffer;

      const qint64 file2BytesRead = file2.read(file2Buffer.data(), 16384);

      if (-1 == file1BytesRead) {
        break;
      }

      if (file1Buffer != file2Buffer) {
        break;
      }

      if (file1.atEnd() && !file2.atEnd()) {
        break;
      }

      if (file1.atEnd() && file2.atEnd()) {
        equivalent = true;
      }
    }
  }

  return equivalent;
}
