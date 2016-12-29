#ifndef KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_
#define KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_

#include "src/utility/pimpl.h"
#include <QObject>
#include <QStringList>
#include <QString>

namespace Kryvos {

inline namespace Cryptography {

class Crypto : public QObject {
  Q_OBJECT

 public:
  /*!
   * \brief Manager Constructs the Manager class
   * \param parent
   */
  explicit Crypto(QObject* parent = nullptr);

  /*!
   * \brief ~Manager Destroys the Manager class
   */
  virtual ~Crypto();

 signals:
  /*!
   * \brief progress Emitted when the cipher operation progress changes
   * \param path String containing path of the file to update progress on
   * \param percent Integer representing the current percent
   */
  void progress(const QString& filePath, qint64 percent);

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation should be displayed to the user
   * \param message String containing the information message to display
   */
  void statusMessage(const QString& message);

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message String containing the error message to display
   * \param filePath String containing the file path which encountered an error
   */
  void errorMessage(const QString& message,
                    const QString& filePath = QString{});

  /*!
   * \brief busyStatus Emitted when a cipher operation starts and ends
   * \param busyStatus Boolean representing the busy status
   */
  void busyStatus(bool busyStatus);

 public slots:
  /*!
   * \brief encrypt Executed when a signal is received for encryption with a
   * passphrase, a list of input file paths, and the algorithm name
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePaths List of strings containing the file paths of the
   * files to encrypt
   * \param outputPath String containing output file path
   * \param cipher String representing name of the cipher
   * \param inputKeySize Key size in bits
   * \param modeOfOperation String representing mode of operation
   * \param compress Boolean representing compression mode
   * \param container Boolean representing container mode
   */
  void encrypt(const QString& passphrase,
               const QStringList& inputFilePaths,
               const QString& outputPath = QString{},
               const QString& cipher = QString{"AES"},
               const std::size_t& inputKeySize = std::size_t{128},
               const QString& modeOfOperation = QString{"GCM"},
               const bool compress = true,
               const bool container = true);

  /*!
   * \brief decrypt Executed when a signal is received for decryption with a
   * passphrase and a list of input file paths
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePaths List of strings containing the file paths of
   * the files to decrypt
   * \param outputPath String containing output file path
   */
  void decrypt(const QString& passphrase,
               const QStringList& inputFilePaths,
               const QString& outputPath = QString{});

  /*!
   * \brief abort Executed when a signal is received to set the abort status.
   * The abort status, when set, will stop the execution of the current cipher
   * operation and prevent further cipher operations from starting until it is
   * reset to false. The current cipher operation is abandoned and cannot be
   * continued.
   */
  void abort();

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
  void stop(const QString& filePath);

 private:
  class CryptoPrivate;
  pimpl<CryptoPrivate> m;
};

}

}

#endif // KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_
