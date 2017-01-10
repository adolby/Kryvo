#include "src/cryptography/Crypto.hpp"
#include "src/cryptography/State.hpp"
#include "src/cryptography/BotanCrypto.hpp"
#include "src/cryptography/constants.h"
#include "src/utility/pimpl_impl.h"
#include <memory>

#if defined(_MSC_VER)
#include "src/utility/make_unique.h"
#endif

class Kryvos::Crypto::CryptoPrivate {
 public:
  /*!
   * \brief CryptoPrivate Constructs the Crypto private implementation
   */
  CryptoPrivate();

  /*!
   * \brief errorMessage Returns error message
   * \return String containing error message
   */
  QString errorMessage(const int index) const;

  std::unique_ptr<State> state;

  std::unique_ptr<BotanCrypto> botanCrypto;

  // The list of status messages that can be displayed to the user
  const QStringList messages;
};

Kryvos::Crypto::Crypto(QObject* parent)
  : QObject{parent} {
  // Subscribe to provider's signals
  connect(m->botanCrypto.get(), &BotanCrypto::fileProgress,
          this, &Crypto::fileProgress);
  connect(m->botanCrypto.get(), &BotanCrypto::archiveProgress,
          this, &Crypto::progress);
  connect(m->botanCrypto.get(), &BotanCrypto::statusMessage,
          this, &Crypto::statusMessage);
  connect(m->botanCrypto.get(), &BotanCrypto::errorMessage,
          this, &Crypto::errorMessage);
}

Kryvos::Crypto::~Crypto() {
}

void Kryvos::Crypto::encrypt(const QString& passphrase,
                              const QStringList& inputFilePaths,
                              const QString& outputPath,
                              const QString& cipher,
                              const std::size_t& inputKeySize,
                              const QString& modeOfOperation,
                              const bool compress,
                              const bool container) {
  m->state->busy(true);
  emit busyStatus(m->state->isBusy());

  const auto& keySize = [&inputKeySize] {
    auto size = std::size_t{128};

    if (inputKeySize > 0) {
      size = inputKeySize;
    }

    return size;
  }();

  m->botanCrypto->encrypt(m->state.get(), passphrase, inputFilePaths,
                          outputPath, cipher, keySize, modeOfOperation,
                          compress, container);

  m->state->reset();

  m->state->busy(false);
  emit busyStatus(m->state->isBusy());
}

void Kryvos::Crypto::decrypt(const QString& passphrase,
                              const QStringList& inputFilePaths,
                              const QString& outputPath) {
  m->state->busy(true);
  emit busyStatus(m->state->isBusy());

  m->botanCrypto->decrypt(m->state.get(), passphrase, inputFilePaths,
                          outputPath);

  m->state->reset();

  m->state->busy(false);
  emit busyStatus(m->state->isBusy());
}

void Kryvos::Crypto::abort() {
  if (m->state->isBusy()) {
    m->state->abort(true);
  }
}

void Kryvos::Crypto::pause(bool pause) {
  m->state->pause(pause);
}

void Kryvos::Crypto::stop(const QString& filePath) {
  if (m->state->isBusy()) {
    m->state->stop(filePath, true);
  }
}

Kryvos::Crypto::CryptoPrivate::CryptoPrivate()
  : state{std::make_unique<State>()},
    botanCrypto{std::make_unique<BotanCrypto>()} {
}
