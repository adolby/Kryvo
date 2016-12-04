#include "src/settings/Settings.hpp"
#include "src/utility/pimpl_impl.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QFile>
#include <QDir>
#include <QStringBuilder>
#include <QCoreApplication>

class Settings::SettingsPrivate {
 public:
  SettingsPrivate();

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
  QString cipher;
  std::size_t keySize;
  QString modeOfOperation;
  bool compressionMode;
  QString lastDirectory;
  QString styleSheetPath;
};

Settings::Settings()
{
  m->importSettings();
}

Settings::~Settings()
{
  m->exportSettings();
}

void Settings::position(const QPoint& position)
{
  m->position = position;
}

QPoint Settings::position() const
{
  return m->position;
}

void Settings::maximized(const bool maximized)
{
  m->maximized = maximized;
}

bool Settings::maximized() const
{
  return m->maximized;
}

void Settings::size(const QSize& size)
{
  m->size = size;
}

QSize Settings::size() const
{
  return m->size;
}

void Settings::cipher(const QString& cipherName)
{
  m->cipher = cipherName;
}

QString Settings::cipher() const
{
  return m->cipher;
}

void Settings::keySize(const std::size_t& keySize)
{
  m->keySize = keySize;
}

std::size_t Settings::keySize() const
{
  return m->keySize;
}

void Settings::modeOfOperation(const QString& modeOfOperation)
{
  m->modeOfOperation = modeOfOperation;
}

QString Settings::modeOfOperation() const
{
  return m->modeOfOperation;
}

void Settings::compressionMode(const bool compress)
{
  m->compressionMode = compress;
}

bool Settings::compressionMode() const
{
  return m->compressionMode;
}

void Settings::lastDirectory(const QString& directory)
{
  m->lastDirectory = directory;
}

QString Settings::lastDirectory() const
{
  return m->lastDirectory;
}

QString Settings::styleSheetPath() const
{
  return m->styleSheetPath;
}

Settings::SettingsPrivate::SettingsPrivate()
  : maximized{false}, keySize{128}, compressionMode{true}
{}

void Settings::SettingsPrivate::importSettings()
{
#if defined(Q_OS_MAC)
  const auto settingsPath = QCoreApplication::applicationDirPath() %
                            QDir::separator() %
                            "settings.json";
#else
  const auto settingsPath = QStringLiteral("settings.json");
#endif

  QFile settingsFile{settingsPath};
  auto fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen)
  {
    auto settingsData = settingsFile.readAll();

    auto settingsDoc = QJsonDocument::fromJson(settingsData);
    auto settings = settingsDoc.object();

    auto positionObject = settings[QStringLiteral("position")].toObject();
    auto xObject = static_cast<QJsonValue>(positionObject[QStringLiteral("x")]);
    auto yObject = static_cast<QJsonValue>(positionObject[QStringLiteral("y")]);
    position = QPoint{xObject.toInt(200), yObject.toInt(200)};

    auto maximizedObject =
      static_cast<QJsonValue>(settings[QStringLiteral("maximized")]);
    maximized = maximizedObject.toBool(false);
    if (!maximized)
    {
      auto sizeObject = settings["size"].toObject();
      auto widthObject =
        static_cast<QJsonValue>(sizeObject[QStringLiteral("width")]);
      auto heightObject =
        static_cast<QJsonValue>(sizeObject[QStringLiteral("height")]);
      size = QSize{widthObject.toInt(800), heightObject.toInt(600)};
    }

    lastDirectory = settings[QStringLiteral("lastDirectory")].toString();

    auto cipherObject =
      static_cast<QJsonValue>(settings[QStringLiteral("cipher")]);
    cipher = cipherObject.toString(QStringLiteral("AES"));

    auto keySizeObject =
      static_cast<QJsonValue>(settings[QStringLiteral("keySize")]);
    keySize = static_cast<std::size_t>(keySizeObject.toInt(128));

    auto modeOfOperationObject =
      static_cast<QJsonValue>(settings[QStringLiteral("modeOfOperation")]);
    modeOfOperation = modeOfOperationObject.toString(QStringLiteral("GCM"));

    auto compressionMode =
        static_cast<QJsonValue>(settings[QStringLiteral("compressionMode")]);
    compressionMode = modeOfOperationObject.toBool(true);

    auto styleObject =
        static_cast<QJsonValue>(settings[QStringLiteral("styleSheetPath")]);
    styleSheetPath = styleObject.toString(QStringLiteral("kryvos.qss"));
  }
  else
  { // Settings file couldn't be opened, so use defaults
    position = QPoint{200, 200};
    size = QSize{800, 600};
    cipher = QStringLiteral("AES");
    keySize = 128;
    modeOfOperation = QStringLiteral("GCM");
    compressionMode = true;
    styleSheetPath = QStringLiteral("kryvos.qss");
  }
}

void Settings::SettingsPrivate::exportSettings() const
{
#if defined(Q_OS_MAC)
  const auto settingsPath = QCoreApplication::applicationDirPath() %
                            QDir::separator() %
                            "settings.json";
#else
  const auto settingsPath = QStringLiteral("settings.json");
#endif

  QSaveFile settingsFile{settingsPath};
  settingsFile.setDirectWriteFallback(true);
  auto fileOpen = settingsFile.open(QIODevice::WriteOnly);

  if (fileOpen)
  {
    auto settings = QJsonObject{};

    auto positionObject = QJsonObject{};
    positionObject[QStringLiteral("x")] = position.x();
    positionObject[QStringLiteral("y")] = position.y();

    settings[QStringLiteral("position")] = positionObject;
    settings[QStringLiteral("maximized")] = maximized;

    if (!maximized)
    {
      auto sizeObject = QJsonObject{};
      sizeObject[QStringLiteral("width")] = size.width();
      sizeObject[QStringLiteral("height")] = size.height();
      settings[QStringLiteral("size")] = sizeObject;
    }

    settings[QStringLiteral("lastDirectory")] = lastDirectory;
    settings[QStringLiteral("cipher")] = cipher;
    settings[QStringLiteral("keySize")] = static_cast<int>(keySize);
    settings[QStringLiteral("modeOfOperation")] = modeOfOperation;
    settings[QStringLiteral("compressionMode")] = compressionMode;
    settings[QStringLiteral("styleSheetPath")] = styleSheetPath;

    auto settingsDoc = QJsonDocument{settings};
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}
