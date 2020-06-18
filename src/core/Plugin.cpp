#include "Plugin.hpp"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>

namespace Kryvo {

QString pluginAttributeFromMetaData(const QJsonObject& metaData,
                                    const QString& attributeName) {
  if (attributeName.isEmpty()) {
    return QString();
  }

  const QJsonValue metaDataValue = metaData.value(QStringLiteral("MetaData"));

  const QJsonObject metaDataObject = metaDataValue.toObject();

  if (metaDataObject.isEmpty()) {
    return QString();
  }

  const QJsonValue value = metaDataObject.value(attributeName);

  if (value.isUndefined()) {
    return QString();
  }

  const QString attribute = value.toString();

  return attribute;
}

} // namespace Kryvo

Kryvo::Plugin::Plugin() = default;

Kryvo::Plugin::Plugin(QObject* instance, const QJsonObject& metaData)
  : m_instance(instance), m_metaData(metaData),
    m_name(pluginAttributeFromMetaData(metaData, QStringLiteral("Name"))),
    m_category(pluginAttributeFromMetaData(metaData,
                                           QStringLiteral("Category"))) {
}

Kryvo::Plugin::Plugin(const QStaticPlugin& staticPlugin)
  : m_instance(staticPlugin.instance()), m_metaData(staticPlugin.metaData()),
    m_name(pluginAttributeFromMetaData(staticPlugin.metaData(),
                                       QStringLiteral("Name"))),
    m_category(pluginAttributeFromMetaData(staticPlugin.metaData(),
                                           QStringLiteral("Category"))) {
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

QString Kryvo::Plugin::category() const {
  return m_category;
}
