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

 public:
  explicit CryptoPrivate(DispatcherState* ds);

  DispatcherState* state{nullptr};

  QHash<QString, CryptoProviderInterface*> availableProviders;

  CryptoProviderInterface* provider{nullptr};
};

Kryvo::CryptoPrivate::CryptoPrivate(DispatcherState* ds)
  : state(ds) {
}

Kryvo::Crypto::Crypto(DispatcherState* state, QObject* parent)
  : QObject(parent),
    d_ptr(std::make_unique<CryptoPrivate>(state)) {
  loadProviders();
}

Kryvo::Crypto::~Crypto() = default;

void Kryvo::Crypto::loadProviders() {
  Q_D(Crypto);

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

        connect(plugin, SIGNAL(fileProgress(QString,QString,qint64)),
                this, SIGNAL(fileProgress(QString,QString,qint64)));

        connect(plugin, SIGNAL(statusMessage(QString)),
                this, SIGNAL(statusMessage(QString)));

        connect(plugin, SIGNAL(errorMessage(QString,QString)),
                this, SIGNAL(errorMessage(QString,QString)));

        d->availableProviders.insert(pluginName, cp);
      }
    }
  }

  if (!d->availableProviders.isEmpty()) {
    if (d->availableProviders.contains(QStringLiteral("BOTAN"))) {
      d->provider = d->availableProviders.value(QStringLiteral("BOTAN"));
    } else {
//      for (CryptoProviderInterface* cp : d->availableProviders) {
//        d->provider = cp;
//        break;
//      }
    }
  }
}

bool Kryvo::Crypto::encryptFile(const QString& passphrase,
                                const QString& inputFilePath,
                                const QString& outputPath,
                                const QString& cipher,
                                std::size_t inputKeySize,
                                const QString& modeOfOperation,
                                bool compress) {
  Q_D(Crypto);

  const std::size_t keySize = [&inputKeySize]() {
    std::size_t size = 128;

    if (inputKeySize > 0) {
      size = inputKeySize;
    }

    return size;
  }();

  if (!d->provider) {
    return false;
  }

  const QDir outputDir(outputPath);

  // Create output path if it doesn't exist
  if (!outputDir.exists()) {
    outputDir.mkpath(outputPath);
  }

  const QFileInfo inputFileInfo(inputFilePath);
  const QString& inFilePath = inputFileInfo.absoluteFilePath();

  const QString& outPath = outputDir.exists() ?
                           outputDir.absolutePath() :
                           inputFileInfo.absolutePath();

  const QString& outFilePath =
    QString(outPath % QStringLiteral("/") % inputFileInfo.fileName() %
            Kryvo::Constants::kDot % Kryvo::Constants::kEncryptedFileExtension);

  const bool success = d->provider->encrypt(d->state, passphrase, inFilePath,
                                            outFilePath, cipher, keySize,
                                            modeOfOperation, compress);

  return success;
}

bool Kryvo::Crypto::decryptFile(const QString& passphrase,
                                const QString& inputFilePath,
                                const QString& outputPath) {
  Q_D(Crypto);

  if (!d->provider) {
    return false;
  }

  const QDir outputDir(outputPath);

  // Create output path if it doesn't exist
  if (!outputDir.exists()) {
    outputDir.mkpath(outputPath);
  }

  const QFileInfo inputFileInfo(inputFilePath);
  const QString& outPath = outputDir.exists() ?
                           outputDir.absolutePath() :
                           inputFileInfo.absolutePath();

  const QString& inFilePath = inputFileInfo.absoluteFilePath();

  const QString& outFilePath = QString(outPath % QStringLiteral("/") %
                                       inputFileInfo.fileName());

  const bool success = d->provider->decrypt(d->state, passphrase, inFilePath,
                                            outFilePath);

  return success;
}

void Kryvo::Crypto::encrypt(const QString& passphrase,
                            const QString& inputFileName,
                            const QString& outputPath, const QString& cipher,
                            std::size_t inputKeySize,
                            const QString& modeOfOperation,
                            bool compress) {
  const bool encryptSuccess = encryptFile(passphrase, inputFileName,
                                          outputPath, cipher, inputKeySize,
                                          modeOfOperation, compress);
}

void Kryvo::Crypto::decrypt(const QString& passphrase,
                            const QString& inputFileName,
                            const QString& outputPath) {
  const bool decryptSuccess = decryptFile(passphrase, inputFileName,
                                          outputPath);
}
