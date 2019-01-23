#ifndef KRYVO_CRYPTOGRAPHY_BOTANPROVIDER_HPP_
#define KRYVO_CRYPTOGRAPHY_BOTANPROVIDER_HPP_

#include "cryptography/CryptoProviderInterface.hpp"
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

#include <QSaveFile>
#include <QFile>
#include <QObject>
#include <QString>
#include <memory>

namespace Kryvo {

class BotanProviderPrivate;

class BotanProvider : public QObject,
                      public CryptoProviderInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "app.kryvo.CryptoProviderInterface" FILE "botan.json")
  Q_INTERFACES(Kryvo::CryptoProviderInterface)
  DECLARE_PRIVATE(BotanProvider)
  std::unique_ptr<BotanProviderPrivate> const d_ptr;

 public:
  explicit BotanProvider(QObject* parent = nullptr);
  ~BotanProvider() override;

 signals:
  void fileCompleted(std::size_t id);

  void fileFailed(std::size_t id);

  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param id ID representing file to update progress on
   * \param task String containing task name
   * \param percent Integer representing the current progress as a percent
   */
  void fileProgress(std::size_t id, const QString& task,
                    qint64 percentProgress) override;

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation should be displayed to the user
   * \param message String containing the information message to display
   */
  void statusMessage(const QString& message) override;

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message String containing the error message to display
   * \param filePath String containing the file path which encountered an error
   */
  void errorMessage(const QString& message, const QString& filePath) override;

 public:
  void init(DispatcherState* state) override;

 /*!
  * \brief encrypt Encrypt a file
  * \param passphrase String representing the user-entered passphrase
  * \param inFilePath String containing the file path of the file to encrypt
  * \param outputPath String containing output file path
  * \param cipher String representing name of the cipher
  * \param inputKeySize Key size in bits
  * \param modeOfOperation String representing mode of operation
  * \param compress Boolean representing compression mode
  */
  bool encrypt(std::size_t id,
               const QString& passphrase,
               const QString& inFilePath,
               const QString& outputPath,
               const QString& cipher,
               std::size_t keySize,
               const QString& modeOfOperation,
               bool compress) override;

  /*!
   * \brief decrypt Decrypt a file. The algorithm is determined from
   * the file header.
   * \param passphrase String representing the user-entered passphrase
   * \param inFilePath Strings containing the file path of the file to decrypt
   * \param outFilePath String containing output file path
   */
  bool decrypt(std::size_t id,
               const QString& passphrase,
               const QString& inFilePath,
               const QString& outFilePath,
               const QString& algorithmNameString,
               const QString& keySizeString,
               const QString& pbkdfSaltString,
               const QString& keySaltString,
               const QString& ivSaltString) override;

  /*!
   * \brief qObject Provide a constant cost QObject conversion
   * \return
   */
  QObject* qObject() override;
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_BOTANPROVIDER_HPP_
