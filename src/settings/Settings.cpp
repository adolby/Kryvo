#include "src/settings/Settings.hpp"
#include "src/utility/pimpl_impl.h"
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringBuilder>
#include <QCoreApplication>

const QStringList kDefaultPaths =
  QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
const QString kDefaultPath = QString{kDefaultPaths.first() %
                                     QDir::separator()};

class Kryvos::Settings::SettingsPrivate {
 public:
  SettingsPrivate();

  /*!
   * \brief importSettings Imports settings from the local settings file
   */
  void importSettings();

  /*!
   * \brief exportSettings Exports settings to the local settings file
   */
  void exportSettings() const;

  QPoint position;
  bool maximized;
  QSize size;
  QString cipher;
  std::size_t keySize;
  QString modeOfOperation;
  bool compressionMode;
  bool containerMode;
  QString outputPath;
  QString lastOpenPath;
  QString styleSheetPath;
};

Kryvos::Settings::Settings() {
  m->importSettings();
}

Kryvos::Settings::~Settings() {
  m->exportSettings();
}

void Kryvos::Settings::position(const QPoint& position) {
  m->position = position;
}

QPoint Kryvos::Settings::position() const {
  return m->position;
}

void Kryvos::Settings::maximized(const bool maximized) {
  m->maximized = maximized;
}

bool Kryvos::Settings::maximized() const {
  return m->maximized;
}

void Kryvos::Settings::size(const QSize& size) {
  m->size = size;
}

QSize Kryvos::Settings::size() const {
  return m->size;
}

void Kryvos::Settings::cipher(const QString& cipherName) {
  m->cipher = cipherName;
}

QString Kryvos::Settings::cipher() const {
  return m->cipher;
}

void Kryvos::Settings::keySize(const std::size_t& keySize) {
  m->keySize = keySize;
}

std::size_t Kryvos::Settings::keySize() const {
  return m->keySize;
}

void Kryvos::Settings::modeOfOperation(const QString& modeOfOperation) {
  m->modeOfOperation = modeOfOperation;
}

QString Kryvos::Settings::modeOfOperation() const {
  return m->modeOfOperation;
}

void Kryvos::Settings::compressionMode(const bool compress) {
  m->compressionMode = compress;
}

bool Kryvos::Settings::compressionMode() const {
  return m->compressionMode;
}

void Kryvos::Settings::containerMode(const bool container) {
  m->containerMode = container;
}

bool Kryvos::Settings::containerMode() const {
  return m->containerMode;
}

void Kryvos::Settings::outputPath(const QString& path) {
  m->outputPath = path;
}

QString Kryvos::Settings::outputPath() const {
  return m->outputPath;
}

void Kryvos::Settings::lastOpenPath(const QString& path) {
  m->lastOpenPath = path;
}

QString Kryvos::Settings::lastOpenPath() const {
  return m->lastOpenPath;
}

QString Kryvos::Settings::styleSheetPath() const {
  return m->styleSheetPath;
}

Kryvos::Settings::SettingsPrivate::SettingsPrivate()
  : maximized{false}, keySize{std::size_t{128}}, compressionMode{true},
    outputPath{kDefaultPath}, lastOpenPath{kDefaultPath} {
}

void Kryvos::Settings::SettingsPrivate::importSettings() {
#if defined(Q_OS_MAC)
  const auto& settingsPath = QCoreApplication::applicationDirPath() %
                             QDir::separator() % "settings.json";
#else
  const auto& settingsPath = QStringLiteral("settings.json");
#endif

  QFile settingsFile{settingsPath};
  const auto fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen) {
    const auto& settingsData = settingsFile.readAll();

    const auto& settingsDoc = QJsonDocument::fromJson(settingsData);
    const auto& settings = settingsDoc.object();

    const auto& positionObject =
      settings[QStringLiteral("position")].toObject();
    position = QPoint{positionObject[QStringLiteral("x")].toInt(100),
                      positionObject[QStringLiteral("y")].toInt(100)};

    maximized = settings[QStringLiteral("maximized")].toBool(false);

    if (!maximized) {
      const auto& sizeObject = settings["size"].toObject();
      size = QSize{sizeObject[QStringLiteral("width")].toInt(800),
                   sizeObject[QStringLiteral("height")].toInt(600)};
    }

    cipher = settings[QStringLiteral("cipher")].toString(QStringLiteral("AES"));

    keySize =
      static_cast<std::size_t>(settings[QStringLiteral("keySize")].toInt(128));

    const auto& modeOfOperationObject =
      static_cast<QJsonValue>(settings[QStringLiteral("modeOfOperation")]);
    modeOfOperation = modeOfOperationObject.toString(QStringLiteral("GCM"));

    compressionMode = settings[QStringLiteral("compressionMode")].toBool(true);

    containerMode = settings[QStringLiteral("containerMode")].toBool(true);

    outputPath = settings[QStringLiteral("outputPath")].toString(kDefaultPath);

    lastOpenPath =
      settings[QStringLiteral("lastOpenPath")].toString(kDefaultPath);

    const auto& styleObject =
        static_cast<QJsonValue>(settings[QStringLiteral("styleSheetPath")]);
    styleSheetPath = styleObject.toString(QStringLiteral("kryvos.qss"));
  }
  else { // Settings file couldn't be opened, so use defaults
    position = QPoint{100, 100};
    maximized = false;
    size = QSize{800, 600};
    cipher = QStringLiteral("AES");
    keySize = std::size_t{128};
    modeOfOperation = QStringLiteral("GCM");
    compressionMode = true;
    containerMode = true;
    outputPath = kDefaultPath;
    lastOpenPath = kDefaultPath;
    styleSheetPath = QStringLiteral("kryvos.qss");
  }
}

void Kryvos::Settings::SettingsPrivate::exportSettings() const {
#if defined(Q_OS_MAC)
  const auto& settingsPath = QCoreApplication::applicationDirPath() %
                             QDir::separator() %
                            "settings.json";
#else
  const auto& settingsPath = QStringLiteral("settings.json");
#endif

  QSaveFile settingsFile{settingsPath};
  settingsFile.setDirectWriteFallback(true);
  const auto fileOpen = settingsFile.open(QIODevice::WriteOnly);

  if (fileOpen) {
    auto settings = QJsonObject{};

    auto positionObject = QJsonObject{};
    positionObject[QStringLiteral("x")] = position.x();
    positionObject[QStringLiteral("y")] = position.y();

    settings[QStringLiteral("position")] = positionObject;
    settings[QStringLiteral("maximized")] = maximized;

    if (!maximized) {
      auto sizeObject = QJsonObject{};
      sizeObject[QStringLiteral("width")] = size.width();
      sizeObject[QStringLiteral("height")] = size.height();
      settings[QStringLiteral("size")] = sizeObject;
    }

    settings[QStringLiteral("cipher")] = cipher;
    settings[QStringLiteral("keySize")] = static_cast<int>(keySize);
    settings[QStringLiteral("modeOfOperation")] = modeOfOperation;
    settings[QStringLiteral("compressionMode")] = compressionMode;
    settings[QStringLiteral("containerMode")] = containerMode;

    auto addPathSeparator = [] (const QString& inPath) {
      const QString& cleanedPath = QDir::cleanPath(inPath);

      const QFileInfo cleanedPathInfo{cleanedPath};

      QString outPath = cleanedPath;

      if (cleanedPathInfo.isDir() && !cleanedPathInfo.isRoot()) {
        outPath = cleanedPath % QDir::separator();
      }

      return outPath;
    };

    settings[QStringLiteral("outputPath")] =
      outputPath.isEmpty() ? kDefaultPath : addPathSeparator(outputPath);
    settings[QStringLiteral("lastOpenPath")] =
      lastOpenPath.isEmpty() ? kDefaultPath : addPathSeparator(lastOpenPath);
    settings[QStringLiteral("styleSheetPath")] =
      addPathSeparator(styleSheetPath);

    const auto& settingsDoc = QJsonDocument{settings};
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}
