#ifndef KRYVO_ARCHIVE_ARCHIVER_HPP_
#define KRYVO_ARCHIVE_ARCHIVER_HPP_

#include "utility/pimpl.h"
#include <QObject>
#include <QStringList>
#include <memory>

namespace Kryvo {

class ArchiverPrivate;

class Archiver : public QObject {
  Q_OBJECT
  DECLARE_PRIVATE(Archiver)
  std::unique_ptr<ArchiverPrivate> const d_ptr;

 public:
  explicit Archiver(QObject* parent = nullptr);

  ~Archiver() override;

 signals:
  /*!
   * \brief fileProgress Emitted when the file operation progress changes
   * \param filePath String containing file path
   * \param task Sting containing task name
   * \param percentProgress Integer representing the current progress as a
   * percent
   */
  void fileProgress(const QString& filePath, const QString& task,
                    qint64 percentProgress);

  /*!
   * \brief statusMessage Emitted when a message about the current
   * operation is produced
   * \param message String containing the information message
   */
  void statusMessage(const QString& message);

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message String containing the error message to display
   * \param filePath String containing the file path which encountered an error
   */
  void errorMessage(const QString& message,
                    const QString& filePath = QString());

 public slots:
  void compressFile(const QString& inFilePath,
                    const QString& outFilePath = QString{});

  void decompressFile(const QString& inFilePath, const QString& outFilePath);

  void archive(const QStringList& inFilePaths);

  void extract(const QString& inFilePath);
};

} // namespace Kryvo

#endif // KRYVO_ARCHIVE_ARCHIVER_HPP_
