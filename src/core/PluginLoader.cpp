#include "PluginLoader.hpp"
#include "Plugin.hpp"
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

  QHash<QString, QObject*> loadedCryptoProviders;
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
    QObject* instance = plugin.instance();

    if (instance) {
      loadedCryptoProviders.insert(pluginName, instance);
    }
  }

  QDir pluginsDir(QCoreApplication::applicationDirPath());

#if defined(Q_OS_MACOS)
  pluginsDir.cdUp();
  pluginsDir.cd(QStringLiteral("PlugIns/cryptography/"));
#endif

  pluginsDir.cd(QStringLiteral("plugins/cryptography/"));

  pluginsDir.setFilter(QDir::Files);

  QDirIterator it(pluginsDir, QDirIterator::Subdirectories);

  while (it.hasNext()) {
    it.next();

    const QFileInfo& fileInfo = it.fileInfo();

    const QString& filePath = fileInfo.absoluteFilePath();

    const bool isLibrary = QLibrary::isLibrary(filePath);

    if (isLibrary) {
      const Plugin& plugin = loadPluginFromFile(filePath);

      const QString& pluginName = plugin.name();

      if (plugin.instance()) {
        loadedCryptoProviders.insert(pluginName, plugin.instance());
      }
    }
  }

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

QHash<QString, QObject*> Kryvo::PluginLoader::cryptoProviders() const {
  Q_D(const PluginLoader);

  return d->loadedCryptoProviders;
}
