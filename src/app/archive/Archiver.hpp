#ifndef KRYVO_ARCHIVE_ARCHIVER_HPP_
#define KRYVO_ARCHIVE_ARCHIVER_HPP_

#include "DispatcherState.hpp"
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
  explicit Archiver(DispatcherState* state, QObject* parent = nullptr);

  ~Archiver() override;

 signals:
  void fileCompressed(const QString& filePath);

  void fileDecompressed(const QString& filePath);

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

 public:
  QByteArray compressChunk(const QByteArray& chunk);

  bool compressFiles(const QStringList& inFilePaths,
                     const QString& outPath = QString());

  bool decompressFiles(const QStringList& inFilePaths,
                       const QString& outPath = QString());

  bool compressFile(const QString& inFilePath, const QString& outFilePath);

  bool decompressFile(const QString& inFilePath, const QString& outFilePath);

 public slots:
  void compress(const QStringList& inFilePaths,
                const QString& outPath = QString());

  void decompress(const QStringList& inFilePaths,
                  const QString& outPath = QString());

  void archive(const QStringList& filePaths);

  void extract(const QString& archiveFilePath);
};

} // namespace Kryvo

#endif // KRYVO_ARCHIVE_ARCHIVER_HPP_
