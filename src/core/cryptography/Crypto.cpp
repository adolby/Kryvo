#include "cryptography/Crypto.hpp"
#include "Constants.hpp"
#include <QDir>

class Kryvo::CryptoPrivate {
  Q_DISABLE_COPY(CryptoPrivate)
  Q_DECLARE_PUBLIC(Crypto)

 public:
  explicit CryptoPrivate(Crypto* crypto, DispatcherState* ds);

  bool encryptFile(std::size_t id, const QString& passphrase,
                   const QFileInfo& inputFileInfo,
                   const QFileInfo& outputFileInfo,
                   const QString& cipher, std::size_t inputKeySize,
                   const QString& modeOfOperation, bool compress);

  bool decryptFile(std::size_t id, const QString& passphrase,
                   const QFileInfo& inputFileInfo,
                   const QFileInfo& outputFileInfo,
                   const QString& algorithmNameString,
                   const QString& keySizeString, const QString& pbkdfSaltString,
                   const QString& keySaltString, const QString& ivSaltString);

  Crypto* const q_ptr{nullptr};

  DispatcherState* state{nullptr};

  CryptoProviderInterface* provider{nullptr};
};

Kryvo::CryptoPrivate::CryptoPrivate(Crypto* crypto, DispatcherState* ds)
  : q_ptr(crypto), state(ds) {
}

bool Kryvo::CryptoPrivate::encryptFile(const std::size_t id,
                                       const QString& passphrase,
                                       const QFileInfo& inputFileInfo,
                                       const QFileInfo& outputFileInfo,
                                       const QString& cipher,
                                       const std::size_t keySize,
                                       const QString& modeOfOperation,
                                       const bool compress) {
  Q_Q(Crypto);

  if (!provider) {
    emit q->fileFailed(id);
    emit q->errorMessage(Constants::messages[11], QFileInfo());
    return false;
  }

  return provider->encrypt(id, passphrase, inputFileInfo, outputFileInfo,
                           cipher, keySize, modeOfOperation, compress);
}

bool Kryvo::CryptoPrivate::decryptFile(const std::size_t id,
                                       const QString& passphrase,
                                       const QFileInfo& inputFileInfo,
                                       const QFileInfo& outputFileInfo,
                                       const QString& algorithmNameString,
                                       const QString& keySizeString,
                                       const QString& pbkdfSaltString,
                                       const QString& keySaltString,
                                       const QString& ivSaltString) {
  Q_Q(Crypto);

  if (!provider) {
    emit q->errorMessage(Constants::messages[11], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  return provider->decrypt(id, passphrase, inputFileInfo, outputFileInfo,
                           algorithmNameString, keySizeString, pbkdfSaltString,
                           keySaltString, ivSaltString);
}

Kryvo::Crypto::Crypto(DispatcherState* state, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<CryptoPrivate>(this, state)) {
}

Kryvo::Crypto::~Crypto() = default;

void Kryvo::Crypto::updateProvider(QObject* provider) {
  Q_D(Crypto);

  if (provider) {
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

    QObject::connect(provider, SIGNAL(errorMessage(const QString&,const QFileInfo&)),
                     this, SIGNAL(errorMessage(const QString&,const QFileInfo&)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    d->provider = qobject_cast<CryptoProviderInterface*>(provider);

    d->provider->init(d->state);
  }
}

bool Kryvo::Crypto::encrypt(const std::size_t id, const QString& passphrase,
                            const QFileInfo& inputFileInfo,
                            const QFileInfo& outputFileInfo,
                            const QString& cipher, std::size_t inputKeySize,
                            const QString& modeOfOperation, bool compress) {
  Q_D(Crypto);

  const bool encrypted = d->encryptFile(id, passphrase, inputFileInfo,
                                        outputFileInfo, cipher, inputKeySize,
                                        modeOfOperation, compress);

  return encrypted;
}

bool Kryvo::Crypto::decrypt(const std::size_t id, const QString& passphrase,
                            const QFileInfo& inputFileInfo,
                            const QFileInfo& outputFileInfo,
                            const QString& algorithmNameString,
                            const QString& keySizeString,
                            const QString& pbkdfSaltString,
                            const QString& keySaltString,
                            const QString& ivSaltString) {
  Q_D(Crypto);

  const bool decrypted = d->decryptFile(id, passphrase, inputFileInfo,
                                        outputFileInfo, algorithmNameString,
                                        keySizeString, pbkdfSaltString,
                                        keySaltString, ivSaltString);

  return decrypted;
}
