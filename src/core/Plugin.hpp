#ifndef KRYVO_PLUGIN_HPP_
#define KRYVO_PLUGIN_HPP_

#include <QStaticPlugin>
#include <QJsonObject>
#include <QObject>

namespace Kryvo {

class Plugin {
 public:
  Plugin();
  Plugin(QObject* instance, const QJsonObject& metaData);
  explicit Plugin(const QStaticPlugin& staticPlugin);

  QObject* instance() const;
  QJsonObject metaData() const;
  QString name() const;
  QString category() const;

 private:
  QObject* instance_ = nullptr;
  QJsonObject metaData_;
  QString name_;
  QString category_;
};

} // namespace Kryvo

#endif // KRYVO_PLUGIN_HPP_
