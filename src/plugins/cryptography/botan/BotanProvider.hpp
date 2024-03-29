#ifndef KRYVO_PLUGINS_CRYPTOGRAPHY_BOTANPROVIDER_HPP_
#define KRYVO_PLUGINS_CRYPTOGRAPHY_BOTANPROVIDER_HPP_

#include "cryptography/CryptoProvider.hpp"
#include "cryptography/EncryptFileConfig.hpp"
#include "cryptography/DecryptFileConfig.hpp"
#include "utility/pimpl.h"

#include <QtGlobal>

#if defined(Q_OS_ANDROID)
  #if defined(Q_PROCESSOR_ARM_V7)
#include "botan/android/armv7/botan_all.h"
  #endif
#elif defined(Q_OS_IOS)
#include "botan/iOS/arm64/botan_all.h"
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
#include "botan/windows/mingw/x86_32/botan_all.h"
    #elif defined(_MSC_VER)
#include "botan/windows/msvc/x86_32/botan_all.h"
    #endif
  #endif
#endif

#include <QFileInfo>
#include <QObject>
#include <QString>
#include <memory>

namespace Kryvo {

class BotanProviderPrivate;

class BotanProvider : public QObject,
                      public CryptoProvider {
  Q_OBJECT
  Q_DISABLE_COPY(BotanProvider)
  Q_PLUGIN_METADATA(IID "app.kryvo.CryptoProvider" FILE "botan.json")
  Q_INTERFACES(Kryvo::CryptoProvider)
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
   * \param config Config for this crypto operation
   */
  void errorMessage(const QString& message, const QFileInfo& fileInfo) override;

 public:
  void init(SchedulerState* state) override;

  /*!
   * \brief encrypt Encrypt a file
   * \param id
   * \param config Encrypt file config
   */
   int encrypt(std::size_t id,
               const Kryvo::EncryptFileConfig& config) override;

  /*!
   * \brief decrypt Decrypt a file. The algorithm is determined from
   * the file header.
   * \param id
   * \param config Decrypt file config
   */
  int decrypt(std::size_t id,
              const Kryvo::DecryptFileConfig& config) override;

  /*!
   * \brief qObject Provide a constant cost QObject conversion
   * \return
   */
  QObject* qObject() override;
};

} // namespace Kryvo

#endif // KRYVO_PLUGINS_CRYPTOGRAPHY_BOTANPROVIDER_HPP_
