#ifndef KRYVO_PLUGINLOADER_HPP_
#define KRYVO_PLUGINLOADER_HPP_

#include "Plugin.hpp"
#include "utility/pimpl.h"
#include <QObject>
#include <QString>
#include <QHash>
#include <memory>

namespace Kryvo {

class PluginLoaderPrivate;

class PluginLoader : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(PluginLoader)
  DECLARE_PRIVATE(PluginLoader)
  std::unique_ptr<PluginLoaderPrivate> const d_ptr;

 public:
  explicit PluginLoader(QObject* parent = nullptr);
  ~PluginLoader() override;

 signals:
  void cryptoProvidersLoaded(const QHash<QString, Plugin>& providers);

 public slots:
  void loadPlugins();
  QHash<QString, Plugin> cryptoProviders() const;
};

} // namespace Kryvo

#endif // KRYVO_PLUGINLOADER_HPP_
