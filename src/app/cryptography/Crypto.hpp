#ifndef KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_
#define KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_

#include "DispatcherState.hpp"
#include "utility/pimpl.h"
#include <QObject>
#include <QString>
#include <memory>
#include <queue>

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
   * \param id ID representing the file to update progress on
   * \param task String containing path name
   * \param percentProgress Integer representing the current progress as a
   * percent
   */
  void fileProgress(int id, const QString& task, qint64 percentProgress);

  void fileEncrypted(int id);

  void fileDecrypted(int id);

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

 public slots:
  /*!
   * \brief encrypt Executed when a signal is received for encryption with a
   * passphrase, a list of input file paths, and the algorithm name
   * \param id Pipeline ID
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePath String containing the file path of the file to
   * encrypt
   * \param outputFilePath String containing output file path
   * \param cipher String representing name of the cipher
   * \param keySize Key size in bits
   * \param modeOfOperation String representing mode of operation
   * \param compress Boolean representing compression mode
   */
  void encrypt(int id,
               const QString& passphrase,
               const QString& inputFilePath,
               const QString& outputFilePath,
               const QString& cipher,
               std::size_t keySize,
               const QString& modeOfOperation,
               bool compress);

  /*!
   * \brief decrypt Executed when a signal is received for decryption with a
   * passphrase and an input file path
   * \param id Pipeline ID
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePath String containing the file path of the file to
   * decrypt
   * \param outputFilePath String containing output file path
   * \param algorithmNameString Algorithm name
   * \param keySizeString Key size as a string
   * \param pbkdfSaltString PBKDF salt
   * \param keySaltString Key salt
   * \param ivSaltString Initialization vector salt
   */
  void decrypt(int id,
               const QString& passphrase,
               const QString& inputFilePath,
               const QString& outputFilePath,
               const QString& algorithmNameString,
               const QString& keySizeString,
               const QString& pbkdfSaltString,
               const QString& keySaltString,
               const QString& ivSaltString);
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_
