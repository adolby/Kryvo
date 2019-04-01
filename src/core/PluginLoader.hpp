#ifndef KRYVO_PLUGINLOADER_HPP_
#define KRYVO_PLUGINLOADER_HPP_

#include "utility/pimpl.h"
#include <QObject>
#include <QString>
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
  void cryptoProviderChanged(QObject* plugin);

 public slots:
  void loadPlugins();
  QObject* cryptoProvider() const;
};

} // namespace Kryvo

#endif // KRYVO_PLUGINLOADER_HPP_
