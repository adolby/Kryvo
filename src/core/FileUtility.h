#ifndef KRYVO_FILEUTILITY_H_
#define KRYVO_FILEUTILITY_H_

#include <QSaveFile>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QMap>
#include <QDebug>

namespace Kryvo {

QMap<QString, QString> readHeader(QFile* file) {
  QMap<QString, QString> headerData;

  // Read metadata from file

  // Read line but skip \n
  auto readLine = [](QFile* file) {
    if (file) {
      QByteArray line = file->readLine();
      return line.replace(QByteArrayLiteral("\n"), QByteArrayLiteral(""));
    }

    return QByteArray();
  };

  int fileVersion = -1;

  QString algorithmNameString;
  QString keySizeString;
  QString compressString;

  QString pbkdfSaltString;
  QString keySaltString;
  QString ivSaltString;

  const QByteArray& headerString = readLine(file);

  while (file->canReadLine()) {
    const QByteArray& line = readLine(file);

    if (QByteArrayLiteral("------------------------------------") == line) {
      break;
    }

    const QString& lineString = QString(line);

    const QList<QString>& keyValuePair = lineString.split(QStringLiteral(": "));

    const QString& key = keyValuePair.at(0);
    const QString& value = keyValuePair.at(1);

    qDebug() << Q_FUNC_INFO << "Reading header: Key:" << key << "Value:" <<
        value;

    headerData.insert(key, value);
  }

  return headerData;
}

void writeHeader(QSaveFile* file,
                 const QMap<QByteArray, QByteArray>& headerData) {
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
}

} // namespace Kryvo

#endif // KRYVO_FILEUTILITY_H_
