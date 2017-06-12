#ifndef KRYVOS_ARCHIVE_ARCHIVER_HPP_
#define KRYVOS_ARCHIVE_ARCHIVER_HPP_

#include "quazip.h"
#include <QObject>
#include <QStringList>

namespace Kryvos {

class Archiver : public QObject {
  Q_OBJECT

 public:
  explicit Archiver(QObject* parent = nullptr);

  bool compressFiles(const QString& fileCompressed,
                     const QStringList& encryptedFiles,
                     const QStringList& originalFiles);
  QStringList extractDir(const QString& fileCompressed,
                         const QString& dir);

 signals:
  /*!
   * \brief progress Emitted when the archive operation progress changes
   * \param filePath String containing file path
   * \param task Sting containing task name
   * \param percentProgress Integer representing the current progress as a
   * percent
   */
  void progress(const QString& filePath, const QString& task,
                const qint64 percentProgress);

 private:
  qint64 copyFileData(QIODevice& inFile, QIODevice& outFile,
                      const QString& filePath, const qint64 fileSize,
                      const QString& task);
  qint64 compressFile(QuaZip* zip, const QString& fileName,
                      const QString& fileDest, const QString& originalFilePath);
  QStringList extractDir(QuaZip& zip, const QString& dir);
  qint64 extractFile(QuaZip* zip, const QString& fileName,
                     const QString& fileDestination = QString{});
};

}

#endif // KRYVOS_ARCHIVE_ARCHIVER_HPP_
