#ifndef KRYVO_ARCHIVE_ARCHIVER_HPP_
#define KRYVO_ARCHIVE_ARCHIVER_HPP_

#include "DispatcherState.hpp"
#include "utility/pimpl.h"
#include <QObject>
#include <QFileInfo>
#include <vector>
#include <memory>

namespace Kryvo {

class ArchiverPrivate;

class Archiver : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(Archiver)
  DECLARE_PRIVATE(Archiver)
  std::unique_ptr<ArchiverPrivate> const d_ptr;

 public:
  explicit Archiver(DispatcherState* state, QObject* parent = nullptr);

  ~Archiver() override;

 signals:
  void fileCompleted(std::size_t id);
  void fileFailed(std::size_t id);

  /*!
   * \brief fileProgress Emitted when the file operation progress changes
   * \param id ID representing file
   * \param task Sting containing task name
   * \param percentProgress Integer representing the current progress as a
   * percent
   */
  void fileProgress(std::size_t id, const QString& task,
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
   * \param fileInfo File that encountered an error
   */
  void errorMessage(const QString& message, const QFileInfo& fileInfo);

 public slots:
  void compress(std::size_t id, const QFileInfo& inputFileInfo,
                const QFileInfo& outputFileInfo);

  void decompress(std::size_t id, const QFileInfo& inputFileInfo,
                  const QFileInfo& outputFileInfo);

//  void archive(const std::vector<QFileInfo>& inputFiles);

//  void extract(const QFileInfo& archiveFileInfo);
};

} // namespace Kryvo

#endif // KRYVO_ARCHIVE_ARCHIVER_HPP_
