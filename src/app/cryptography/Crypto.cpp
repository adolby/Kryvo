#include "cryptography/Crypto.hpp"
#include "cryptography/CryptoState.hpp"
#include "cryptography/CryptoProviderInterface.hpp"
#include "archive/Archiver.hpp"
#include "constants.h"
#include <QPluginLoader>
#include <QLibrary>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>

#include <QDebug>

class Kryvo::CryptoPrivate {
  Q_DISABLE_COPY(CryptoPrivate)

 public:
  CryptoPrivate();

  /*!
   * \brief errorMessage Returns error message
   * \return String containing error message
   */
  QString errorMessage(int index) const;

  CryptoState state;

  QHash<QString, CryptoProviderInterface*> availableProviders;

  CryptoProviderInterface* provider{nullptr};

  // The list of status messages that can be displayed to the user
  const QStringList messages;
};

Kryvo::CryptoPrivate::CryptoPrivate() = default;

Kryvo::Crypto::Crypto(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<CryptoPrivate>()) {
  loadProviders();
}

Kryvo::Crypto::~Crypto() = default;

void Kryvo::Crypto::loadProviders() {
  Q_D(Crypto);

  QDir pluginsDir(qApp->applicationDirPath());

  pluginsDir.cdUp();

  pluginsDir.cd(QStringLiteral("plugins"));

  const QFileInfoList fileInfoList = pluginsDir.entryInfoList(QDir::Files);

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

        connect(plugin, SIGNAL(fileProgress(const QString&, const QString&,
                                            const qint64)),
                this, SIGNAL(fileProgress(const QString&, const QString&,
                                          const qint64)));

        connect(plugin, SIGNAL(statusMessage(const QString&)),
                this, SIGNAL(statusMessage(const QString&)));

        connect(plugin, SIGNAL(errorMessage(const QString&,
                                            const QString&)),
                this, SIGNAL(errorMessage(const QString&,
                                          const QString&)));

        d->availableProviders.insert(pluginName, cp);
      }
    }
  }

  if (!d->availableProviders.isEmpty()) {
    if (d->availableProviders.contains(QStringLiteral("BOTAN"))) {
      d->provider = d->availableProviders.value(QStringLiteral("BOTAN"));
    }
    else {
      for (CryptoProviderInterface* cp : d->availableProviders) {
        d->provider = cp;
        break;
      }
    }
  }
}

void Kryvo::Crypto::encrypt(const QString& passphrase,
                            const QStringList& inputFilePaths,
                            const QString& outputPath, const QString& cipher,
                            const std::size_t inputKeySize,
                            const QString& modeOfOperation,
                            const bool compress, const bool container) {
  Q_D(Crypto);

  if ( !d->provider ) {
    return;
  }

  d->state.busy(true);
  emit busyStatus(d->state.isBusy());

  const std::size_t keySize = [&inputKeySize]() {
    std::size_t size = 128;

    if (inputKeySize > 0) {
      size = inputKeySize;
    }

    return size;
  }();

  d->provider->encrypt(&d->state, passphrase, inputFilePaths, outputPath,
                       cipher, keySize, modeOfOperation, compress, container);

  d->state.reset();

  d->state.busy(false);
  emit busyStatus(d->state.isBusy());
}

void Kryvo::Crypto::decrypt(const QString& passphrase,
                            const QStringList& inputFilePaths,
                            const QString& outputPath) {
  Q_D(Crypto);

  if ( !d->provider ) {
    return;
  }

  d->state.busy(true);
  emit busyStatus(d->state.isBusy());

  d->provider->decrypt(&d->state, passphrase, inputFilePaths, outputPath);

  d->state.reset();

  d->state.busy(false);
  emit busyStatus(d->state.isBusy());
}

void Kryvo::Crypto::abort() {
  Q_D(Crypto);

  if (d->state.isBusy()) {
    d->state.abort(true);
  }
}

void Kryvo::Crypto::pause(const bool pause) {
  Q_D(Crypto);

  d->state.pause(pause);
}

void Kryvo::Crypto::stop(const QString& filePath) {
  Q_D(Crypto);

  if (d->state.isBusy()) {
    d->state.stop(filePath, true);
  }
}
