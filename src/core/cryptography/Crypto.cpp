#include "cryptography/Crypto.hpp"
#include "Constants.hpp"
#include <QDir>

class Kryvo::CryptoPrivate {
  Q_DISABLE_COPY(CryptoPrivate)
  Q_DECLARE_PUBLIC(Crypto)

 public:
  explicit CryptoPrivate(Crypto* crypto, DispatcherState* ds);

  bool encryptFile(std::size_t id, const QString& cryptoProvider,
                   const QString& compressionFormat, const QString& passphrase,
                   const QFileInfo& inputFileInfo,
                   const QFileInfo& outputFileInfo, const QString& cipher,
                   std::size_t inputKeySize, const QString& modeOfOperation);

  bool decryptFile(std::size_t id, const QString& cryptoProvider,
                   const QString& passphrase, const QFileInfo& inputFileInfo,
                   const QFileInfo& outputFileInfo,
                   const QByteArray& algorithmNameByteArray,
                   const QByteArray& keySizeByteArray,
                   const QByteArray& pbkdfSaltByteArray,
                   const QByteArray& keySaltByteArray,
                   const QByteArray& ivSaltByteArray);

  Crypto* const q_ptr{nullptr};

  DispatcherState* state{nullptr};

  QHash<QString, CryptoProviderInterface*> providers;
};

Kryvo::CryptoPrivate::CryptoPrivate(Crypto* crypto, DispatcherState* ds)
  : q_ptr(crypto), state(ds) {
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

  CryptoProviderInterface* provider = providers.value(cryptoProvider);

  if (!provider) {
    emit q->fileFailed(id);
    emit q->errorMessage(Constants::kMessages[11], QFileInfo());
    return false;
  }

  return provider->encrypt(id, compressionFormat, passphrase, inputFileInfo,
                           outputFileInfo, cipher, keySize, modeOfOperation);
}

bool Kryvo::CryptoPrivate::decryptFile(const std::size_t id,
                                       const QString& cryptoProvider,
                                       const QString& passphrase,
                                       const QFileInfo& inputFileInfo,
                                       const QFileInfo& outputFileInfo,
                                       const QByteArray& algorithmNameByteArray,
                                       const QByteArray& keySizeByteArray,
                                       const QByteArray& pbkdfSaltByteArray,
                                       const QByteArray& keySaltByteArray,
                                       const QByteArray& ivSaltByteArray) {
  Q_Q(Crypto);

  CryptoProviderInterface* provider = providers.value(cryptoProvider);

  if (!provider) {
    emit q->errorMessage(Constants::kMessages[11], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  return provider->decrypt(id, passphrase, inputFileInfo, outputFileInfo,
                           algorithmNameByteArray, keySizeByteArray,
                           pbkdfSaltByteArray, keySaltByteArray,
                           ivSaltByteArray);
}

Kryvo::Crypto::Crypto(DispatcherState* state, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<CryptoPrivate>(this, state)) {
}

Kryvo::Crypto::~Crypto() = default;

void Kryvo::Crypto::updateProviders(
  const QHash<QString, QObject*>& loadedProviders) {
  Q_D(Crypto);

  d->providers.clear();

  auto end = loadedProviders.cend();

  for (auto providerIterator = loadedProviders.cbegin();
       providerIterator != end;
       ++providerIterator) {
    QObject* provider = providerIterator.value();

    QObject::connect(provider, SIGNAL(fileCompleted(std::size_t)),
                     this, SIGNAL(fileCompleted(std::size_t)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider, SIGNAL(fileFailed(std::size_t)),
                     this, SIGNAL(fileFailed(std::size_t)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider,
                     SIGNAL(fileProgress(std::size_t,const QString&,qint64)),
                     this,
                     SIGNAL(fileProgress(std::size_t,const QString&,qint64)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider, SIGNAL(statusMessage(const QString&)),
                     this, SIGNAL(statusMessage(const QString&)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider,
                     SIGNAL(errorMessage(const QString&,const QFileInfo&)),
                     this,
                     SIGNAL(errorMessage(const QString&,const QFileInfo&)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    CryptoProviderInterface* cryptoProvider =
      qobject_cast<CryptoProviderInterface*>(provider);

    cryptoProvider->init(d->state);

    d->providers.insert(providerIterator.key(), cryptoProvider);
  }
}

bool Kryvo::Crypto::encrypt(const std::size_t id, const QString& cryptoProvider,
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

bool Kryvo::Crypto::decrypt(const std::size_t id, const QString& cryptoProvider,
                            const QString& passphrase,
                            const QFileInfo& inputFileInfo,
                            const QFileInfo& outputFileInfo,
                            const QByteArray& algorithmNameByteArray,
                            const QByteArray& keySizeByteArray,
                            const QByteArray& pbkdfSaltByteArray,
                            const QByteArray& keySaltByteArray,
                            const QByteArray& ivSaltByteArray) {
  Q_D(Crypto);

  const bool decrypted = d->decryptFile(id, cryptoProvider, passphrase,
                                        inputFileInfo, outputFileInfo,
                                        algorithmNameByteArray,
                                        keySizeByteArray, pbkdfSaltByteArray,
                                        keySaltByteArray, ivSaltByteArray);

  return decrypted;
}
