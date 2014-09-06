#include "settings/Settings.hpp"
#include "utility/make_unique.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QFile>

class Settings::SettingsPrivate {
 public:
  explicit SettingsPrivate();

  /*!
   * \brief importSettings Imports settings from the local settings file:
   * the last application size and position, the last directory opened by the
   * file open dialog, the last algorithm set, and the stylesheet path for
   * theming.
   */
  void importSettings();

  /*!
   * \brief exportSettings Exports settings to the local settings file: the
   * application size and position, the last directory opened by the open
   * dialog, and the stylesheet path for theming.
   */
  void exportSettings() const;

  QPoint position;
  bool maximized;
  QSize size;
  QString lastDirectory;
  QString lastAlgorithmName;
  QString styleSheetPath;
};

Settings::Settings() :
  pimpl{make_unique<SettingsPrivate>()}
{
  pimpl->importSettings();
}

Settings::~Settings()
{
  pimpl->exportSettings();
}

void Settings::position(const QPoint& position)
{
  Q_ASSERT(pimpl);

  pimpl->position = position;
}

QPoint Settings::position()
{
  Q_ASSERT(pimpl);

  return pimpl->position;
}

void Settings::maximized(bool maximized)
{
  Q_ASSERT(pimpl);

  pimpl->maximized = maximized;
}

bool Settings::maximized()
{
  Q_ASSERT(pimpl);

  return pimpl->maximized;
}

void Settings::size(const QSize& size)
{
  Q_ASSERT(pimpl);

  pimpl->size = size;
}

QSize Settings::size()
{
  Q_ASSERT(pimpl);

  return pimpl->size;
}

void Settings::lastAlgorithm(const QString& algorithmName)
{
  Q_ASSERT(pimpl);

  pimpl->lastAlgorithmName = algorithmName;
}

QString Settings::lastAlgorithm()
{
  Q_ASSERT(pimpl);

  return pimpl->lastAlgorithmName;
}

void Settings::lastDirectory(const QString& directory)
{
  Q_ASSERT(pimpl);

  pimpl->lastDirectory = directory;
}

QString Settings::lastDirectory()
{
  Q_ASSERT(pimpl);

  return pimpl->lastDirectory;
}

QString Settings::styleSheetPath()
{
  Q_ASSERT(pimpl);

  return pimpl->styleSheetPath;
}

Settings::SettingsPrivate::SettingsPrivate()
  : maximized{false}
{}

void Settings::SettingsPrivate::importSettings()
{
  QFile settingsFile{"settings.json"};
  auto fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen)
  {
    auto settingsData = settingsFile.readAll();

    auto settingsDoc = QJsonDocument::fromJson(settingsData);
    auto settings = settingsDoc.object();

    auto positionObject = settings["position"].toObject();
    auto x = static_cast<QJsonValue>(positionObject["x"]);
    auto y = static_cast<QJsonValue>(positionObject["y"]);
    position = QPoint{x.toInt(200), y.toInt(200)};

    maximized = settings["maximized"].toBool();
    if (!maximized)
    {
      auto sizeObject = settings["size"].toObject();
      auto width = static_cast<QJsonValue>(sizeObject["width"]);
      auto height = static_cast<QJsonValue>(sizeObject["height"]);
      size = QSize{width.toInt(800), height.toInt(600)};
    }

    lastDirectory = settings["lastDirectory"].toString();

    auto algorithm = static_cast<QJsonValue>(settings["lastAlgorithmName"]);
    lastAlgorithmName = algorithm.toString("AES-128/GCM");

    auto style = static_cast<QJsonValue>(settings["styleSheetPath"]);
    styleSheetPath = style.toString("default/kryvos.qss");
  }
  else
  { // Settings file couldn't be opened, so use defaults
    position = QPoint{200, 200};
    size = QSize{800, 600};
    styleSheetPath = "default/kryvos.qss";
    lastAlgorithmName = "AES-128/GCM";
  }
}

void Settings::SettingsPrivate::exportSettings() const
{
  QFile settingsFile{"settings.json"};
  auto fileOpen = settingsFile.open(QIODevice::WriteOnly);

  if (fileOpen)
  {
    auto settings = QJsonObject{};

    auto positionObject = QJsonObject{};
    positionObject["x"] = position.x();
    positionObject["y"] = position.y();

    settings["position"] = positionObject;
    settings["maximized"] = maximized;

    if (!maximized)
    {
      auto sizeObject = QJsonObject{};
      sizeObject["width"] = size.width();
      sizeObject["height"] = size.height();
      settings["size"] = sizeObject;
    }

    settings["lastDirectory"] = lastDirectory;
    settings["lastAlgorithmName"] = lastAlgorithmName;
    settings["styleSheetPath"] = styleSheetPath;

    auto settingsDoc = QJsonDocument{settings};
    settingsFile.write(settingsDoc.toJson());
  }
}
