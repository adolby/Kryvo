#include "PluginLoader.hpp"
#include "cryptography/CryptoProviderInterface.hpp"
#include <QPluginLoader>
#include <QStaticPlugin>
#include <QLibrary>
#include <QJsonObject>
#include <QJsonArray>
#include <QDirIterator>
#include <QVector>
#include <QCoreApplication>

Q_IMPORT_PLUGIN(BotanProvider)

class Kryvo::PluginLoaderPrivate {
  Q_DISABLE_COPY(PluginLoaderPrivate)
  Q_DECLARE_PUBLIC(PluginLoader)

 public:
  explicit PluginLoaderPrivate(PluginLoader* loader);

  Plugin loadPluginFromFile(const QString& filePath);

  void loadPlugins();

  PluginLoader* const q_ptr{nullptr};

  QHash<QString, Plugin> loadedCryptoProviders;
};

Kryvo::PluginLoaderPrivate::PluginLoaderPrivate(PluginLoader* loader)
  : q_ptr(loader) {
}

Kryvo::Plugin
Kryvo::PluginLoaderPrivate::loadPluginFromFile(const QString& filePath) {
  QPluginLoader loader(filePath);

  QObject* plugin = loader.instance();

  const QJsonObject& metaData = loader.metaData();

  return Plugin(plugin, metaData);
}

void Kryvo::PluginLoaderPrivate::loadPlugins() {
  Q_Q(PluginLoader);

  const QVector<QStaticPlugin>& staticPlugins =
    QPluginLoader::staticPlugins();

  for (const QStaticPlugin& staticPlugin : staticPlugins) {
    const Plugin plugin(staticPlugin);

    const QString& pluginName = plugin.name();

    if (plugin.instance()) {
      if (QStringLiteral("Crypto") == plugin.category()) {
        loadedCryptoProviders.insert(pluginName, plugin);
      }
    }
  }

#if !defined(Q_OS_IOS)
#if defined(Q_OS_ANDROID)
  QDir pluginsDir(QCoreApplication::applicationDirPath());
#elif defined(Q_OS_MACOS)
  QDir pluginsDir(QCoreApplication::applicationDirPath());
  pluginsDir.cdUp();
  pluginsDir.cd(QStringLiteral("PlugIns/cryptography/"));
#else
  QDir pluginsDir(QCoreApplication::applicationDirPath() +
                  QStringLiteral("plugins/cryptography/"));
#endif

  if (pluginsDir.exists()) {
    pluginsDir.setFilter(QDir::Files);

    QDirIterator it(pluginsDir, QDirIterator::Subdirectories);

    while (it.hasNext()) {
      it.next();

      const QFileInfo& fileInfo = it.fileInfo();

      const QString& filePath = fileInfo.absoluteFilePath();

      const bool isLibrary = QLibrary::isLibrary(filePath);

      if (isLibrary) {
        const Plugin& plugin = loadPluginFromFile(filePath);

        if (plugin.instance()) {
          if (QStringLiteral("Crypto") == plugin.category()) {
            loadedCryptoProviders.insert(plugin.name(), plugin);
          }
        }
      }
    }
  }
#endif

  Q_ASSERT_X(loadedCryptoProviders.size() > 0, "loadProviders",
             "At least one provider plugin is required");

  emit q->cryptoProvidersChanged(loadedCryptoProviders);
}

Kryvo::PluginLoader::PluginLoader(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<PluginLoaderPrivate>(this)) {
}

Kryvo::PluginLoader::~PluginLoader() = default;

void Kryvo::PluginLoader::loadPlugins() {
  Q_D(PluginLoader);

  d->loadPlugins();
}

QHash<QString, Kryvo::Plugin> Kryvo::PluginLoader::cryptoProviders() const {
  Q_D(const PluginLoader);

  return d->loadedCryptoProviders;
}
