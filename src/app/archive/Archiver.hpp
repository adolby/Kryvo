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
  void fileCompressed(int id, const QString& outputFilePath);

  void fileDecompressed(int id, const QString& outputFilePath);

  /*!
   * \brief fileProgress Emitted when the file operation progress changes
   * \param id ID representing file
   * \param task Sting containing task name
   * \param percentProgress Integer representing the current progress as a
   * percent
   */
  void fileProgress(int id, const QString& task, qint64 percentProgress);

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
  void compress(int id, const QString& inputFilePath,
                const QString& outputFilePath);

  void decompress(int id, const QString& inputFilePath,
                  const QString& outputFilePath);

  void archive(const QStringList& filePaths);

  void extract(const QString& archiveFilePath);
};

} // namespace Kryvo

#endif // KRYVO_ARCHIVE_ARCHIVER_HPP_
