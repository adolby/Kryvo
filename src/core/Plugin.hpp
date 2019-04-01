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

 private:
  QObject* m_instance = nullptr;
  QJsonObject m_metaData;
  QString m_name;
};

}

#endif // KRYVO_PLUGIN_HPP_
