#include "cryptography/Crypto.hpp"
#include "Constants.hpp"
#include <QDir>

class Kryvo::CryptoPrivate {
  Q_DISABLE_COPY(CryptoPrivate)
  Q_DECLARE_PUBLIC(Crypto)

 public:
  explicit CryptoPrivate(Crypto* crypto, DispatcherState* ds);

  bool encryptFile(std::size_t id, const QString& passphrase,
                   const QString& inputFilePath, const QString& outputPath,
                   const QString& cipher, std::size_t inputKeySize,
                   const QString& modeOfOperation, bool compress);

  bool decryptFile(std::size_t id, const QString& passphrase,
                   const QString& inputFilePath, const QString& outputPath,
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
                                       const QString& inputFilePath,
                                       const QString& outputFilePath,
                                       const QString& cipher,
                                       const std::size_t keySize,
                                       const QString& modeOfOperation,
                                       const bool compress) {
  Q_Q(Crypto);

  if (!provider) {
    emit q->fileFailed(id);
    emit q->errorMessage(Constants::messages[11]);
    return false;
  }

  return provider->encrypt(id, passphrase, inputFilePath, outputFilePath,
                           cipher, keySize, modeOfOperation, compress);
}

bool Kryvo::CryptoPrivate::decryptFile(const std::size_t id,
                                       const QString& passphrase,
                                       const QString& inputFilePath,
                                       const QString& outputFilePath,
                                       const QString& algorithmNameString,
                                       const QString& keySizeString,
                                       const QString& pbkdfSaltString,
                                       const QString& keySaltString,
                                       const QString& ivSaltString) {
  Q_Q(Crypto);

  if (!provider) {
    emit q->errorMessage(Constants::messages[11]);
    emit q->fileFailed(id);
    return false;
  }

  return provider->decrypt(id, passphrase, inputFilePath, outputFilePath,
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
                     SIGNAL(fileProgress(std::size_t,QString,qint64)),
                     this,
                     SIGNAL(fileProgress(std::size_t,QString,qint64)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider, SIGNAL(statusMessage(QString)),
                     this, SIGNAL(statusMessage(QString)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    QObject::connect(provider, SIGNAL(errorMessage(QString,QString)),
                     this, SIGNAL(errorMessage(QString,QString)),
                     static_cast<Qt::ConnectionType>(Qt::DirectConnection |
                                                     Qt::UniqueConnection));

    d->provider = qobject_cast<CryptoProviderInterface*>(provider);

    d->provider->init(d->state);
  }
}

bool Kryvo::Crypto::encrypt(const std::size_t id, const QString& passphrase,
                            const QString& inputFilePath,
                            const QString& outputFilePath,
                            const QString& cipher, std::size_t inputKeySize,
                            const QString& modeOfOperation, bool compress) {
  Q_D(Crypto);

  const bool encrypted = d->encryptFile(id, passphrase, inputFilePath,
                                        outputFilePath, cipher, inputKeySize,
                                        modeOfOperation, compress);

  return encrypted;
}

bool Kryvo::Crypto::decrypt(const std::size_t id, const QString& passphrase,
                            const QString& inputFilePath,
                            const QString& outputPath,
                            const QString& algorithmNameString,
                            const QString& keySizeString,
                            const QString& pbkdfSaltString,
                            const QString& keySaltString,
                            const QString& ivSaltString) {
  Q_D(Crypto);

  const bool decrypted = d->decryptFile(id, passphrase, inputFilePath,
                                        outputPath, algorithmNameString,
                                        keySizeString, pbkdfSaltString,
                                        keySaltString, ivSaltString);

  return decrypted;
}
