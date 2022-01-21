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

Plugin::Plugin() = default;

Plugin::Plugin(QObject* instance, const QJsonObject& metaData)
  : instance_(instance), metaData_(metaData),
    name_(pluginAttributeFromMetaData(metaData, QStringLiteral("Name"))),
    category_(pluginAttributeFromMetaData(metaData,
                                          QStringLiteral("Category"))) {
}

Plugin::Plugin(const QStaticPlugin& staticPlugin)
  : instance_(staticPlugin.instance()), metaData_(staticPlugin.metaData()),
    name_(pluginAttributeFromMetaData(staticPlugin.metaData(),
                                      QStringLiteral("Name"))),
    category_(pluginAttributeFromMetaData(staticPlugin.metaData(),
                                          QStringLiteral("Category"))) {
}

QObject* Plugin::instance() const {
  return instance_;
}

QJsonObject Plugin::metaData() const {
  return metaData_;
}

QString Plugin::name() const {
  return name_;
}

QString Plugin::category() const {
  return category_;
}

} // namespace Kryvo
