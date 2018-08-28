#ifndef KRYVO_ARCHIVE_ARCHIVER_HPP_
#define KRYVO_ARCHIVE_ARCHIVER_HPP_

#include <QObject>
#include <QStringList>

namespace Kryvo {

class Archiver : public QObject {
  Q_OBJECT

 public:
  explicit Archiver(QObject* parent = nullptr);

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
  void compressedFilePath(const QString& filePath);
  void extractedFilePath(const QString& filePath);

 public slots:
  void compress(const QStringList& inFilePaths,
                const QString& outFilePath = QString{});
  void extract(const QString& inFilePath, const QString& outFilePath);
};

} // namespace Kryvo

#endif // KRYVO_ARCHIVE_ARCHIVER_HPP_
