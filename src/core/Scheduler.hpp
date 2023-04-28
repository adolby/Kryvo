#ifndef KRYVO_SCHEDULER_HPP_
#define KRYVO_SCHEDULER_HPP_

#include "cryptography/EncryptFileConfig.hpp"
#include "cryptography/DecryptFileConfig.hpp"
#include "cryptography/EncryptConfig.hpp"
#include "cryptography/DecryptConfig.hpp"
#include "archive/CompressFileConfig.hpp"
#include "archive/DecompressFileConfig.hpp"
#include "Plugin.hpp"
#include "utility/pimpl.h"
#include <QObject>
#include <QFileInfo>
#include <QHash>
#include <QStringList>
#include <QString>
#include <functional>
#include <vector>
#include <memory>

namespace Kryvo {

struct Pipeline {
  std::vector<std::function<void(std::size_t)>> stages;
  std::size_t stage = 0;
  QFileInfo inputFilePath;
};

class SchedulerPrivate;

class Scheduler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(Scheduler)
  DECLARE_PRIVATE(Scheduler)
  std::unique_ptr<SchedulerPrivate> const d_ptr;

 public:
  explicit Scheduler(QObject* parent = nullptr);

  ~Scheduler() override;

 signals:
  void fileCompleted(std::size_t id);

  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param fileInfo File to update progress on
   * \param task String containing path name
   * \param progressValue Integer representing the current progress as a percent
   */
  void fileProgress(const QFileInfo& fileInfo, const QString& task,
                    qint64 percentProgress);

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation is produced
   * \param message String containing the information message to display
   */
  void statusMessage(const QString& message);

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message String containing the error message to display
   * \param filePath File that encountered an error
   */
  void errorMessage(const QString& message, const QFileInfo& fileInfo);

  void running(bool run);

  void compressFile(std::size_t id, const Kryvo::CompressFileConfig& config);

  void decompressFile(std::size_t id,
                      const Kryvo::DecompressFileConfig& config);

  void encryptFile(std::size_t id,
                   const Kryvo::EncryptFileConfig& config);

  void decryptFile(std::size_t id, const Kryvo::DecryptFileConfig& config);

  void cryptoProvidersLoaded(const QHash<QString, Plugin>& providers);

 public slots:
  /*!
   * \brief encrypt Executed when a signal is received for encryption with a
   * passphrase, a list of input file paths, and the algorithm name
   * \param config Encrypt config
   * \param inputFiles List of files to encrypt
   * \param outputPath Output path
   */
  void encrypt(const EncryptConfig& config,
               const std::vector<QFileInfo>& inputFiles,
               const QDir& outputPath);

  /*!
   * \brief decrypt Executed when a signal is received for decryption with a
   * passphrase and a list of input file paths
   * \param config Decrypt config
   * \param inputFiles List of files to decrypt
   * \param outputPath Output path
   */
  void decrypt(const DecryptConfig& config,
               const std::vector<QFileInfo>& inputFiles,
               const QDir& outputPath);

  /*!
   * \brief cancel Executed when a signal is received to set the cancel status.
   * The cancel status, when set, will stop the execution of the current cipher
   * operation and prevent further cipher operations from starting until it is
   * reset to false. The current cipher operation is abandoned and cannot be
   * continued.
   */
  void cancel();

  /*!
   * \brief pause Executed when a signal is received to set or clear the pause
   * status (via the state of the boolean parameter pause). The pause status, if
   * set to true, will pause the execution of the current cipher operation until
   * it is reset to false. When the pause status is reset to false, the cipher
   * operation that was in progress when the pause was signaled will resume
   * execution.
   * \param pause Boolean representing the pause state
   */
  void pause(bool pause);

  /*!
   * \brief stop Executed when a signal is received to set the stop status for
   * the file path input parameter. The stop status, if set, will skip the input
   * file path in the encrypt/decrypt process.
   * \param filePath String containing a file path
   */
  void stop(const QFileInfo& filePath);

  void processPipeline(std::size_t id);

  void cancelPipeline(std::size_t id);

  void updateFileProgress(std::size_t id, const QString& task,
                          qint64 percentProgress);
};

} // namespace Kryvo

#endif // KRYVO_SCHEDULER_HPP_
