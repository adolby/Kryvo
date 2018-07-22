#ifndef KRYVO_CRYPTOGRAPHY_BOTANPROVIDER_HPP_
#define KRYVO_CRYPTOGRAPHY_BOTANPROVIDER_HPP_

#include "cryptography/CryptoProviderInterface.hpp"
#include "cryptography/CryptoState.hpp"
#include "utility/pimpl.h"

#include <QtGlobal>

#if defined(Q_OS_ANDROID)
#include "botan/android/botan_all.h"
#elif defined(Q_OS_IOS)
#include "botan/iOS/botan_all.h"
#elif defined(Q_OS_MACOS)
#include "botan/macOS/clang/x86_64/botan_all.h"
#elif defined(Q_OS_LINUX)
  #if defined(__clang__)
#include "botan/linux/clang/x86_64/botan_all.h"
  #elif defined(__GNUC__) || defined(__GNUG__)
#include "botan/linux/gcc/x86_64/botan_all.h"
  #endif
#elif defined(Q_OS_WIN)
  #if defined(Q_OS_WIN64)
    #if defined(__GNUC__) || defined(__GNUG__)
#include "botan/windows/mingw/x86_64/botan_all.h"
    #elif defined(_MSC_VER)
#include "botan/windows/msvc/x86_64/botan_all.h"
    #endif
  #else
    #if defined(__GNUC__) || defined(__GNUG__)
#include "botan/windows/mingw/x86/botan_all.h"
    #elif defined(_MSC_VER)
#include "botan/windows/msvc/x86/botan_all.h"
    #endif
  #endif
#endif

#include <QObject>
#include <QString>
#include <fstream>
#include <string>
#include <memory>

namespace Kryvo {

class BotanProviderPrivate;

class BotanProvider : public QObject,
                      public CryptoProviderInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "io.kryvo.CryptoProviderInterface" FILE "botan.json")
  Q_INTERFACES(Kryvo::CryptoProviderInterface)
  DECLARE_PRIVATE(BotanProvider)
  std::unique_ptr<BotanProviderPrivate> const d_ptr;

 public:
  explicit BotanProvider(QObject* parent = nullptr);
  ~BotanProvider() Q_DECL_OVERRIDE;

 signals:
  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param path String containing path of the file to update progress on
   * \param task String containing task name
   * \param percent Integer representing the current progress as a percent
   */
  void fileProgress(const QString& filePath, const QString& task,
                    const qint64 percentProgress) Q_DECL_OVERRIDE;

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation should be displayed to the user
   * \param message String containing the information message to display
   */
  void statusMessage(const QString& message) Q_DECL_OVERRIDE;

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message String containing the error message to display
   * \param filePath String containing the file path which encountered an error
   */
  void errorMessage(const QString& message,
                    const QString& filePath = QString()) Q_DECL_OVERRIDE;

 public:
 /*!
  * \brief encrypt Encrypt a list of files
  * \param state Encryption process state
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
  bool encrypt(CryptoState* state,
               const QString& passphrase,
               const QStringList& inputFilePaths,
               const QString& outputPath = QString(),
               const QString& cipher = QString("AES"),
               const std::size_t keySize = 128,
               const QString& modeOfOperation = QString("GCM"),
               const bool compress = true,
               const bool container = true) Q_DECL_OVERRIDE;

  /*!
   * \brief decrypt Decrypt a list of files. The algorithm is determined from
   * the file header.
   * \param state Decryption process state
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePaths List of strings containing the file paths of
   * the files to decrypt
   * \param outputPath String containing output file path
   */
  bool decrypt(CryptoState* state,
               const QString& passphrase,
               const QStringList& inputFilePaths,
               const QString& outputPath) Q_DECL_OVERRIDE;

  /*!
   * \brief encryptFile Encrypts a single file with the input passphrase and
   * algorithm name
   * \param state Encryption process state
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
  bool encryptFile(CryptoState* state,
                   const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputFilePath,
                   const QString& algorithmName,
                   const std::size_t keySize,
                   const bool compress);

  /*!
   * \brief decryptFile Decrypts a single file with the input passphrase and
   * algorithm name
   * \param state Decryption process state
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePath String containing the file path of the file to decrypt
   * \param outputPath String containing output file path
   */
  bool decryptFile(CryptoState* state,
                   const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputFilePath);

  /*!
   * \brief executeCipher Executes a cipher on a file with the a key,
   * initialization vector, and cipher direction
   * \param state Cipher process state
   * \param direction Cipher direction
   * \param inputFilePath String containing the file path of the file to
   * encrypt/decrypt
   * \param pipe Botan pipe for encryption or decryption
   * \param in Input file stream
   * \param out Output file stream
   */
  bool executeCipher(CryptoState* state,
                     Botan::Cipher_Dir direction,
                     const QString& inputFilePath,
                     Botan::Pipe& pipe,
                     std::ifstream& in,
                     std::ofstream& out);

  /*!
   * \brief qObject Provide a constant cost QObject conversion
   * \return
   */
  QObject* qObject() Q_DECL_OVERRIDE;
};

}

#endif // KRYVO_CRYPTOGRAPHY_BOTANPROVIDER_HPP_
