#include "cryptography/Crypto.hpp"
#include "cryptography/CryptoProviderInterface.hpp"
#include "Constants.hpp"
#include <QPluginLoader>
#include <QLibrary>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfoList>
#include <QDir>
#include <QCoreApplication>

class Kryvo::CryptoPrivate {
  Q_DISABLE_COPY(CryptoPrivate)
  Q_DECLARE_PUBLIC(Crypto)

 public:
  explicit CryptoPrivate(Crypto* crypto, DispatcherState* ds);

  void loadProviders();

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

  QHash<QString, CryptoProviderInterface*> availableProviders;

  CryptoProviderInterface* provider{nullptr};
};

Kryvo::CryptoPrivate::CryptoPrivate(Crypto* crypto, DispatcherState* ds)
  : q_ptr(crypto), state(ds) {
  loadProviders();
}

Kryvo::Crypto::Crypto(DispatcherState* state, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<CryptoPrivate>(this, state)) {
}

Kryvo::Crypto::~Crypto() = default;

void Kryvo::CryptoPrivate::loadProviders() {
  Q_Q(Crypto);

  QDir pluginsDir(qApp->applicationDirPath());

#if defined(Q_OS_MACOS)
  pluginsDir.cdUp();
  pluginsDir.cd(QStringLiteral("PlugIns/cryptography/botan/"));
#endif

  pluginsDir.cd(QStringLiteral("plugins/cryptography/botan/"));

  const QFileInfoList& fileInfoList = pluginsDir.entryInfoList(QDir::Files);

  for (const QFileInfo& fileInfo : fileInfoList) {
    const QString& filePath = fileInfo.absoluteFilePath();

    const bool isLibrary = QLibrary::isLibrary(filePath);

    if (isLibrary) {
      QPluginLoader loader(filePath);

      QObject* plugin = loader.instance();

      if (plugin) {
        const QJsonObject& metaData = loader.metaData();

        const QJsonValue& metaDataValue =
            metaData.value(QStringLiteral("MetaData"));

        const QJsonObject& metaDataObject = metaDataValue.toObject();

        const QJsonValue& keys =
            metaDataObject.value(QStringLiteral("Keys"));

        const QJsonArray& keysArray = keys.toArray();

        const QJsonValue& pluginNameValue = keysArray.first();

        const QString& pluginName = pluginNameValue.toString();

        CryptoProviderInterface* cp =
          qobject_cast<CryptoProviderInterface*>(plugin);

        QObject::connect(plugin, SIGNAL(fileCompleted(std::size_t)),
                         q, SIGNAL(fileCompleted(std::size_t)));

        QObject::connect(plugin, SIGNAL(fileFailed(std::size_t)),
                         q, SIGNAL(fileFailed(std::size_t)));

        QObject::connect(plugin,
                         SIGNAL(fileProgress(std::size_t,QString,qint64)),
                         q,
                         SIGNAL(fileProgress(std::size_t,QString,qint64)));

        QObject::connect(plugin, SIGNAL(statusMessage(QString)),
                         q, SIGNAL(statusMessage(QString)));

        QObject::connect(plugin, SIGNAL(errorMessage(QString,QString)),
                         q, SIGNAL(errorMessage(QString,QString)));

        cp->init(state);

        availableProviders.insert(pluginName, cp);
      }
    }
  }

  if (!availableProviders.isEmpty()) {
    if (availableProviders.contains(QStringLiteral("BOTAN"))) {
      provider = availableProviders.value(QStringLiteral("BOTAN"));
    } else {
//      for (CryptoProviderInterface* cp : d->availableProviders) {
//        d->provider = cp;
//        break;
//      }
    }
  }

  Q_ASSERT_X(provider, "loadProviders", "Provider plugin is missing");
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

bool Kryvo::Crypto::encrypt(const std::size_t id, const QString& passphrase,
                            const QString& inputFilePath,
                            const QString& outputFilePath,
                            const QString& cipher, std::size_t inputKeySize,
                            const QString& modeOfOperation, bool compress) {
  Q_D(Crypto);

  const bool encrypted = d->encryptFile(id, passphrase, inputFilePath,
                                        outputFilePath, cipher, inputKeySize,
                                        modeOfOperation, compress);

  Q_ASSERT(encrypted);

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

  Q_ASSERT(decrypted);

  return decrypted;
}
