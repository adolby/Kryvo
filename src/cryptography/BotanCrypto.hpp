#ifndef KRYVOS_CRYPTOGRAPHY_BOTANCRYPTO_HPP_
#define KRYVOS_CRYPTOGRAPHY_BOTANCRYPTO_HPP_

#include "src/cryptography/CryptoState.hpp"

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
#include <QString>
#include <fstream>
#include <string>

class BotanCrypto : public QObject {
  Q_OBJECT

 public:
  /*!
   * \brief BotanCrypto Constructs the BotanCrypto class. Initializes Botan.
   * \param parent
   */
  explicit BotanCrypto(CryptoState* state, QObject* parent = nullptr);

  ~BotanCrypto();

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

 private:
  class BotanCryptoPrivate;
  pimpl<BotanCryptoPrivate> m;
};

#endif // KRYVOS_CRYPTOGRAPHY_BOTANCRYPTO_HPP_
