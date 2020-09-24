#ifndef PIPE_HPP
#define PIPE_HPP

#include <QObject>
#include <QFileInfo>
#include <QString>

namespace Kryvo {

class Pipe : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(Pipe)

 public:
  explicit Pipe(QObject* parent = nullptr);
  virtual ~Pipe() = default;

 signals:
  void fileCompleted(std::size_t id);
  void fileFailed(std::size_t id);

  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param id File identifier to update progress on
   * \param task Task name string
   * \param percentProgress Integer progress percent
   */
  void fileProgress(std::size_t id, const QString& task, qint64 percentProgress);

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation is produced
   * \param message Information message string
   */
  void statusMessage(const QString& message);

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message Error message string
   * \param fileInfo File that encountered an error
   */
  void errorMessage(const QString& message, const QFileInfo& fileInfo);
};

} // namespace Kryvo

#endif // PIPE_HPP
