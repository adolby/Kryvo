#include "Plugin.hpp"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>

namespace Kryvo {

QString pluginNameFromMetaData(const QJsonObject& metaData) {
  const QJsonValue& metaDataValue =
    metaData.value(QStringLiteral("MetaData"));

  const QJsonObject& metaDataObject = metaDataValue.toObject();

  const QJsonValue& keys = metaDataObject.value(QStringLiteral("Keys"));

  const QJsonArray& keysArray = keys.toArray();

  const QJsonValue& pluginNameValue = keysArray.first();

  const QString& pluginName = pluginNameValue.toString();

  return pluginName;
}

}

Kryvo::Plugin::Plugin() = default;

Kryvo::Plugin::Plugin(QObject* instance, const QJsonObject& metaData)
  : m_instance(instance), m_metaData(metaData),
    m_name(pluginNameFromMetaData(metaData)) {
}

Kryvo::Plugin::Plugin(const QStaticPlugin& staticPlugin)
  : m_instance(staticPlugin.instance()), m_metaData(staticPlugin.metaData()),
    m_name(pluginNameFromMetaData(staticPlugin.metaData())) {
}

QObject* Kryvo::Plugin::instance() const {
  return m_instance;
}

QJsonObject Kryvo::Plugin::metaData() const {
  return m_metaData;
}

QString Kryvo::Plugin::name() const {
  return m_name;
}
