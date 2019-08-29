#ifndef KRYVO_FILEUTILITY_H_
#define KRYVO_FILEUTILITY_H_

#include <QSaveFile>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QHash>

namespace Kryvo {

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
