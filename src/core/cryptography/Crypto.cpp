#include "cryptography/Crypto.hpp"
#include "Constants.hpp"
#include "DispatchQueue.hpp"
#include <QDir>
#include <mutex>
#include <shared_mutex>

class Kryvo::CryptoPrivate {
  Q_DISABLE_COPY(CryptoPrivate)
  Q_DECLARE_PUBLIC(Crypto)

 public:
  explicit CryptoPrivate(Crypto* crypto, SchedulerState* s);

  void updateProviders(const QHash<QString, Plugin>& loadedProviders);

  bool encryptFile(std::size_t id, const QString& cryptoProvider,
                   const QString& compressionFormat, const QString& passphrase,
                   const QFileInfo& inputFileInfo,
                   const QFileInfo& outputFileInfo, const QString& cipher,
                   std::size_t inputKeySize, const QString& modeOfOperation);

  bool decryptFile(std::size_t id, const QString& cryptoProvider,
                   const QString& passphrase, const QFileInfo& inputFileInfo,
                   const QFileInfo& outputFileInfo);

  void encrypt(std::size_t id, const QString& cryptoProvider,
               const QString& compressionFormat, const QString& passphrase,
               const QFileInfo& inputFileInfo,
               const QFileInfo& outputFileInfo, const QString& cipher,
               std::size_t inputKeySize, const QString& modeOfOperation);

  void decrypt(std::size_t id, const QString& cryptoProvider,
               const QString& passphrase, const QFileInfo& inputFileInfo,
               const QFileInfo& outputFileInfo);

  Crypto* const q_ptr{nullptr};

  SchedulerState* state{nullptr};

  DispatchQueue queue;

  QHash<QString, CryptoProvider*> providers;
  std::shared_timed_mutex providersMutex;
};

Kryvo::CryptoPrivate::CryptoPrivate(Crypto* crypto, SchedulerState* s)
  : q_ptr(crypto), state(s), providersMutex(){
}

void Kryvo::CryptoPrivate::updateProviders(
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

bool Kryvo::CryptoPrivate::encryptFile(const std::size_t id,
                                       const QString& cryptoProvider,
                                       const QString& compressionFormat,
                                       const QString& passphrase,
                                       const QFileInfo& inputFileInfo,
                                       const QFileInfo& outputFileInfo,
                                       const QString& cipher,
                                       const std::size_t keySize,
                                       const QString& modeOfOperation) {
  Q_Q(Crypto);

  std::shared_lock<std::shared_timed_mutex> lock(providersMutex);

  if (!providers.contains(cryptoProvider)) {
    emit q->fileFailed(id);
    return false;
  }

  CryptoProvider* provider = providers.value(cryptoProvider);

  if (!provider) {
    emit q->fileFailed(id);
    emit q->errorMessage(Constants::kMessages[11], QFileInfo());
    return false;
  }

  return provider->encrypt(id, compressionFormat, passphrase, inputFileInfo,
                           outputFileInfo, cipher, keySize, modeOfOperation);
}

bool Kryvo::CryptoPrivate::decryptFile(
  const std::size_t id, const QString& cryptoProvider,
  const QString& passphrase, const QFileInfo& inputFileInfo,
  const QFileInfo& outputFileInfo) {
  Q_Q(Crypto);

  std::shared_lock<std::shared_timed_mutex> lock(providersMutex);

  if (!providers.contains(cryptoProvider)) {
    emit q->fileFailed(id);
    return false;
  }

  CryptoProvider* provider = providers.value(cryptoProvider);

  if (!provider) {
    emit q->fileFailed(id);
    emit q->errorMessage(Constants::kMessages[11], QFileInfo());
    return false;
  }

  return provider->decrypt(id, passphrase, inputFileInfo, outputFileInfo);
}

void Kryvo::CryptoPrivate::encrypt(const std::size_t id,
                                   const QString& cryptoProvider,
                                   const QString& compressionFormat,
                                   const QString& passphrase,
                                   const QFileInfo& inputFileInfo,
                                   const QFileInfo& outputFileInfo,
                                   const QString& cipher,
                                   const std::size_t keySize,
                                   const QString& modeOfOperation) {
  const auto encryptFunc =
    [this, id, cryptoProvider, compressionFormat, passphrase, inputFileInfo,
     outputFileInfo, cipher, keySize, modeOfOperation]() {
      encryptFile(id, cryptoProvider, compressionFormat, passphrase,
                  inputFileInfo, outputFileInfo, cipher, keySize,
                  modeOfOperation);
    };

  DispatchTask task;
  task.func = encryptFunc;

  queue.enqueue(task);
}

void Kryvo::CryptoPrivate::decrypt(const std::size_t id,
                                   const QString& cryptoProvider,
                                   const QString& passphrase,
                                   const QFileInfo& inputFileInfo,
                                   const QFileInfo& outputFileInfo) {
  const auto decryptFunc =
    [this, id, cryptoProvider, passphrase, inputFileInfo, outputFileInfo]() {
      decryptFile(id, cryptoProvider, passphrase, inputFileInfo,
                  outputFileInfo);
    };

  DispatchTask task;
  task.func = decryptFunc;

  queue.enqueue(task);
}

Kryvo::Crypto::Crypto(SchedulerState* state, QObject* parent)
  : Pipe(parent), d_ptr(std::make_unique<CryptoPrivate>(this, state)) {
}

Kryvo::Crypto::~Crypto() = default;

bool Kryvo::Crypto::encryptFile(const std::size_t id,
                                const QString& cryptoProvider,
                                const QString& compressionFormat,
                                const QString& passphrase,
                                const QFileInfo& inputFileInfo,
                                const QFileInfo& outputFileInfo,
                                const QString& cipher, std::size_t inputKeySize,
                                const QString& modeOfOperation) {
  Q_D(Crypto);

  const bool encrypted = d->encryptFile(id, cryptoProvider, compressionFormat,
                                        passphrase, inputFileInfo,
                                        outputFileInfo, cipher, inputKeySize,
                                        modeOfOperation);

  return encrypted;
}

bool Kryvo::Crypto::decryptFile(const std::size_t id,
                                const QString& cryptoProvider,
                                const QString& passphrase,
                                const QFileInfo& inputFileInfo,
                                const QFileInfo& outputFileInfo) {
  Q_D(Crypto);

  const bool decrypted = d->decryptFile(id, cryptoProvider, passphrase,
                                        inputFileInfo, outputFileInfo);

  return decrypted;
}

void Kryvo::Crypto::updateProviders(
  const QHash<QString, Plugin>& loadedProviders) {
  Q_D(Crypto);

  d->updateProviders(loadedProviders);
}

void Kryvo::Crypto::encrypt(const std::size_t id,
                            const QString& cryptoProvider,
                            const QString& compressionFormat,
                            const QString& passphrase,
                            const QFileInfo& inputFileInfo,
                            const QFileInfo& outputFileInfo,
                            const QString& cipher, std::size_t inputKeySize,
                            const QString& modeOfOperation) {
  Q_D(Crypto);

  d->encrypt(id, cryptoProvider, compressionFormat, passphrase, inputFileInfo,
             outputFileInfo, cipher, inputKeySize, modeOfOperation);
}

void Kryvo::Crypto::decrypt(const std::size_t id,
                            const QString& cryptoProvider,
                            const QString& passphrase,
                            const QFileInfo& inputFileInfo,
                            const QFileInfo& outputFileInfo) {
  Q_D(Crypto);

  d->decrypt(id, cryptoProvider, passphrase, inputFileInfo, outputFileInfo);
}
