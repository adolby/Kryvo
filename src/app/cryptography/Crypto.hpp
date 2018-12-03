#ifndef KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_
#define KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_

#include "DispatcherState.hpp"
#include "utility/pimpl.h"
#include <QObject>
#include <QString>
#include <memory>

namespace Kryvo {

class CryptoPrivate;

class Crypto : public QObject {
  Q_OBJECT
  DECLARE_PRIVATE(Crypto)
  std::unique_ptr<CryptoPrivate> const d_ptr;

 public:
  /*!
   * \brief Crypto Constructs the Crypto class
   * \param parent
   */
  explicit Crypto(DispatcherState* state, QObject* parent = nullptr);

  ~Crypto() override;

 signals:
  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param path String containing path of the file to update progress on
   * \param task String containing path name
   * \param progressValue Integer representing the current progress as a percent
   */
  void fileProgress(const QString& filePath, const QString& task,
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
   * \param filePath String containing the file path which encountered an error
   */
  void errorMessage(const QString& message,
                    const QString& filePath = QString());

  /*!
   * \brief busyStatus Emitted when a cipher operation starts and ends
   * \param busyStatus Boolean representing the busy status
   */
  void busyStatus(bool busyStatus);

 public:
  void loadProviders();
  bool encryptFile(const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputPath = QString(),
                   const QString& cipher = QStringLiteral("AES"),
                   std::size_t inputKeySize = 128,
                   const QString& modeOfOperation = QStringLiteral("GCM"),
                   bool compress = true);

  bool decryptFile(const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputPath = QString());

 public slots:
  /*!
   * \brief encrypt Executed when a signal is received for encryption with a
   * passphrase, a list of input file paths, and the algorithm name
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePath String containing the file path of the file to
   * encrypt
   * \param outputPath String containing output file path
   * \param cipher String representing name of the cipher
   * \param inputKeySize Key size in bits
   * \param modeOfOperation String representing mode of operation
   * \param compress Boolean representing compression mode
   * \param container Boolean representing container mode
   */
  void encrypt(const QString& passphrase,
               const QString& inputFilePath,
               const QString& outputPath = QString(),
               const QString& cipher = QStringLiteral("AES"),
               std::size_t inputKeySize = 128,
               const QString& modeOfOperation = QStringLiteral("GCM"),
               bool compress = true);

  /*!
   * \brief decrypt Executed when a signal is received for decryption with a
   * passphrase and an input file path
   * \param passphrase String representing the user-entered passphrase
   * \param inputFileName String containing the file path of the file to
   * decrypt
   * \param outputPath String containing output file path
   */
  void decrypt(const QString& passphrase,
               const QString& inputFilePath,
               const QString& outputPath = QString());
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_
