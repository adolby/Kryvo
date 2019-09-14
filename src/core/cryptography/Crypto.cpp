#include "cryptography/Crypto.hpp"
#include "cryptography/EncryptFileConfig.hpp"
#include "cryptography/DecryptFileConfig.hpp"
#include "Constants.hpp"
#include "DispatchQueue.hpp"
#include <QDir>
#include <mutex>
#include <shared_mutex>

namespace Kryvo {

class CryptoPrivate {
  Q_DISABLE_COPY(CryptoPrivate)
  Q_DECLARE_PUBLIC(Crypto)

 public:
  explicit CryptoPrivate(Crypto* crypto, SchedulerState* s);

  void updateProviders(const QHash<QString, Plugin>& loadedProviders);

  bool encryptFile(std::size_t id,
                   const Kryvo::EncryptFileConfig& config);

  bool decryptFile(std::size_t id,
                   const Kryvo::DecryptFileConfig& config);

  void encrypt(std::size_t id,
               const Kryvo::EncryptFileConfig& config);

  void decrypt(std::size_t id,
               const Kryvo::DecryptFileConfig& config);

  Crypto* const q_ptr{nullptr};

  SchedulerState* state{nullptr};

  DispatchQueue queue;

  QHash<QString, CryptoProvider*> providers;
  std::shared_timed_mutex providersMutex;
};

CryptoPrivate::CryptoPrivate(Crypto* crypto, SchedulerState* s)
  : q_ptr(crypto), state(s), providersMutex(){
}

void CryptoPrivate::updateProviders(
  const QHash<QString, Plugin>& loadedProviders) {
  Q_Q(Crypto);

  std::lock_guard<std::shared_timed_mutex> lock(providersMutex);

  providers.clear();

  auto end = loadedProviders.cend();

  for (auto providerIterator = loadedProviders.cbegin();
       providerIterator != end;
       ++providerIterator) {
    const Plugin providerPlugin = providerIterator.value();

    QObject* provider = providerPlugin.instance();

    QObject::connect(provider, SIGNAL(fileCompleted(std::size_t)),
                     q_ptr, SIGNAL(fileCompleted(std::size_t)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider, SIGNAL(fileFailed(std::size_t)),
                     q_ptr, SIGNAL(fileFailed(std::size_t)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider,
                     SIGNAL(fileProgress(std::size_t,const QString&,qint64)),
                     q_ptr,
                     SIGNAL(fileProgress(std::size_t,const QString&,qint64)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider, SIGNAL(statusMessage(const QString&)),
                     q_ptr, SIGNAL(statusMessage(const QString&)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider,
                     SIGNAL(errorMessage(const QString&,const QFileInfo&)),
                     q_ptr,
                     SIGNAL(errorMessage(const QString&,const QFileInfo&)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    CryptoProvider* cryptoProvider = qobject_cast<CryptoProvider*>(provider);

    cryptoProvider->init(state);

    providers.insert(providerIterator.key(), cryptoProvider);
  }
}

bool CryptoPrivate::encryptFile(std::size_t id,
                                const Kryvo::EncryptFileConfig& config) {
  Q_Q(Crypto);

  std::shared_lock<std::shared_timed_mutex> lock(providersMutex);

  if (!providers.contains(config.provider)) {
    emit q->fileFailed(id);
    return false;
  }

  CryptoProvider* provider = providers.value(config.provider);

  if (!provider) {
    emit q->fileFailed(id);
    emit q->errorMessage(Constants::messages[11], QFileInfo());
    return false;
  }

  return provider->encrypt(id, config);
}

bool CryptoPrivate::decryptFile(std::size_t id,
                                const Kryvo::DecryptFileConfig& config) {
  Q_Q(Crypto);

  std::shared_lock<std::shared_timed_mutex> lock(providersMutex);

  if (!providers.contains(config.provider)) {
    emit q->fileFailed(id);
    return false;
  }

  CryptoProvider* provider = providers.value(config.provider);

  if (!provider) {
    emit q->fileFailed(id);
    emit q->errorMessage(Constants::messages[11], QFileInfo());
    return false;
  }

  return provider->decrypt(id, config);
}

void CryptoPrivate::encrypt(std::size_t id,
                            const Kryvo::EncryptFileConfig& config) {
  const auto encryptFunc =
    [this, id, config]() {
      encryptFile(id, config);
    };

  DispatchTask task;
  task.func = encryptFunc;

  queue.enqueue(task);
}

void CryptoPrivate::decrypt(std::size_t id,
                            const Kryvo::DecryptFileConfig& config) {
  const auto decryptFunc =
    [this, id, config]() {
      decryptFile(id, config);
    };

  DispatchTask task;
  task.func = decryptFunc;

  queue.enqueue(task);
}

Crypto::Crypto(SchedulerState* state, QObject* parent)
  : Pipe(parent), d_ptr(std::make_unique<CryptoPrivate>(this, state)) {
}

Crypto::~Crypto() = default;

bool Crypto::encryptFile(std::size_t id,
                         const Kryvo::EncryptFileConfig& config) {
  Q_D(Crypto);

  const bool encrypted = d->encryptFile(id, config);

  return encrypted;
}

bool Crypto::decryptFile(std::size_t id,
                         const Kryvo::DecryptFileConfig& config) {
  Q_D(Crypto);

  const bool decrypted = d->decryptFile(id, config);

  return decrypted;
}

void Crypto::updateProviders(
  const QHash<QString, Plugin>& loadedProviders) {
  Q_D(Crypto);

  d->updateProviders(loadedProviders);
}

void Crypto::encrypt(std::size_t id, const Kryvo::EncryptFileConfig& config) {
  Q_D(Crypto);

  d->encrypt(id, config);
}

void Crypto::decrypt(std::size_t id, const Kryvo::DecryptFileConfig& config) {
  Q_D(Crypto);

  d->decrypt(id, config);
}

}
