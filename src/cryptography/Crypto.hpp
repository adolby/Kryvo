#ifndef KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_
#define KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_

#include <QtGlobal>

#if defined(Q_OS_ANDROID)
#include "src/cryptography/botan/android/botan_all.h"
#elif defined(Q_OS_IOS)
#include "src/cryptography/botan/iOS/botan_all.h"
#elif defined(Q_OS_MACX)
#include "src/cryptography/botan/macOS/clang/x86_64/botan_all.h"
#elif defined(Q_OS_LINUX)
  #if defined(__clang__)
#include "src/cryptography/botan/linux/clang/x86_64/botan_all.h"
  #elif defined(__GNUC__) || defined(__GNUG__)
#include "src/cryptography/botan/linux/gcc/x86_64/botan_all.h"
  #endif
#elif defined(Q_OS_WIN)
  #if defined(Q_OS_WIN64)
    #if defined(__GNUC__) || defined(__GNUG__)
#include "src/cryptography/botan/windows/mingw/x86_64/botan_all.h"
    #elif defined(_MSC_VER)
#include "src/cryptography/botan/windows/msvc/x86_64/botan_all.h"
    #endif
  #else
    #if defined(__GNUC__) || defined(__GNUG__)
#include "src/cryptography/botan/windows/mingw/x86/botan_all.h"
    #elif defined(_MSC_VER)
#include "src/cryptography/botan/windows/msvc/x86/botan_all.h"
    #endif
  #endif
#endif

#include "src/utility/pimpl.h"
#include <QObject>
#include <QStringList>
#include <QString>
#include <fstream>
#include <string>

/*!
 * \brief The Crypto class performs encryption and decryption using the Botan
 * library.
 */
class Crypto : public QObject {
  Q_OBJECT

 public:
  /*!
   * \brief Crypto Constructs the Crypto class. Initializes Botan.
   * \param parent
   */
  explicit Crypto(QObject* parent = nullptr);

  /*!
   * \brief ~Crypto Destroys the Crypto class
   */
  virtual ~Crypto();

 signals:
  /*!
   * \brief progress Emitted when the cipher operation progress changes
   * \param path String containing path of file to update progress on
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
   * \param path String containing the file path which encountered an error
   */
  void errorMessage(const QString& message, const QString& path = QString{});

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
   * passphrase and a list of input file paths. The algorithm is determined from
   * the file header.
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
  /*!
   * \brief encryptFile Encrypts a single file with the input passphrase and
   * algorithm name
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePath String representing the file path of the file to
   * encrypt
   * \param outputFilePath String containing the file path of the encrypted
   * file
   * \param algorithmName String representing the name of the algorithm to use
   * for encryption
   * \param keySize Key size
   * \param compress Boolean representing compression mode
   */
  void encryptFile(const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputFilePath,
                   const QString& algorithmName,
                   const std::size_t& keySize,
                   const bool compress);

  /*!
   * \brief decryptFile Decrypts a single file with the input passphrase and
   * algorithm name
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePath String containing the file path of the file to decrypt
   * \param outputPath String containing output path
   */
  void decryptFile(const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputPath);

  /*!
   * \brief executeCipher Executes a cipher on a file with the a key,
   * initialization vector, and cipher direction
   * \param inputFilePath String containing the file path of the file to
   * encrypt/decrypt
   * \param pipe Botan pipe for encryption or decryption
   * \param in Input file stream
   * \param out Output file stream
   */
  void executeCipher(const QString& inputFilePath,
                     Botan::Pipe& pipe,
                     std::ifstream& in,
                     std::ofstream& out);

 private:
  class CryptoPrivate;
  pimpl<CryptoPrivate> m;
};

#endif // KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_
