#ifndef KRYVOS_ARCHIVE_ARCHIVER_HPP_
#define ARCHIVER_HPP

#include "quazip.h"
#include <QObject>
#include <QStringList>

class Archiver : public QObject
{
  Q_OBJECT

 public:
  explicit Archiver(QObject* parent = nullptr);

  bool compressFiles(const QString& fileCompressed,
                     const QStringList& files);
  QStringList extractDir(const QString& fileCompressed,
                         const QString& dir);
  QString extractFile(QIODevice* ioDevice, const QString& fileName,
                      const QString& fileDest);

 signals:
  /*!
   * \brief progress Emitted when the archive operation progress changes
   * \param percent Integer representing the current progress as a percent
   */
  void progress(const qint64 percentProgress);

 private:
  bool copyData(QIODevice& inFile, QIODevice& outFile);
  bool compressFile(QuaZip* zip, const QString& fileName,
                    const QString& fileDest);
  QStringList extractDir(QuaZip& zip, const QString& dir);
  QString extractFile(QuaZip& zip, const QString& fileName,
                      const QString& fileDest);
  bool extractFile(QuaZip* zip, const QString& fileName,
                      const QString& fileDestination);

};

#endif // KRYVOS_ARCHIVE_ARCHIVER_HPP_
