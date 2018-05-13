#include "settings/Settings.hpp"
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringBuilder>
#include <QApplication>

namespace Kryvo {
  const QStringList kDefaultPaths =
    QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
  const QString kDefaultPath = QString{kDefaultPaths.first() %
                                       QDir::separator()};
}

class Kryvo::SettingsPrivate {
  Q_DISABLE_COPY(SettingsPrivate)

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

Kryvo::Settings::Settings()
  : d_ptr{std::make_unique<SettingsPrivate>()} {
  Q_D(Settings);

  d->importSettings();
}

Kryvo::Settings::~Settings() {
  Q_D(Settings);

  d->exportSettings();
}

void Kryvo::Settings::position(const QPoint& position) {
  Q_D(Settings);

  d->position = position;
}

QPoint Kryvo::Settings::position() const {
  Q_D(const Settings);

  return d->position;
}

void Kryvo::Settings::maximized(const bool maximized) {
  Q_D(Settings);

  d->maximized = maximized;
}

bool Kryvo::Settings::maximized() const {
  Q_D(const Settings);

  return d->maximized;
}

void Kryvo::Settings::size(const QSize& size) {
  Q_D(Settings);

  d->size = size;
}

QSize Kryvo::Settings::size() const {
  Q_D(const Settings);

  return d->size;
}

void Kryvo::Settings::cipher(const QString& cipherName) {
  Q_D(Settings);

  d->cipher = cipherName;
}

QString Kryvo::Settings::cipher() const {
  Q_D(const Settings);

  return d->cipher;
}

void Kryvo::Settings::keySize(const std::size_t keySize) {
  Q_D(Settings);

  d->keySize = keySize;
}

std::size_t Kryvo::Settings::keySize() const {
  Q_D(const Settings);

  return d->keySize;
}

void Kryvo::Settings::modeOfOperation(const QString& modeOfOperation) {
  Q_D(Settings);

  d->modeOfOperation = modeOfOperation;
}

QString Kryvo::Settings::modeOfOperation() const {
  Q_D(const Settings);

  return d->modeOfOperation;
}

void Kryvo::Settings::compressionMode(const bool compress) {
  Q_D(Settings);

  d->compressionMode = compress;
}

bool Kryvo::Settings::compressionMode() const {
  Q_D(const Settings);

  return d->compressionMode;
}

void Kryvo::Settings::containerMode(const bool container) {
  Q_D(Settings);

  d->containerMode = container;
}

bool Kryvo::Settings::containerMode() const {
  Q_D(const Settings);

  return d->containerMode;
}

void Kryvo::Settings::outputPath(const QString& path) {
  Q_D(Settings);

  d->outputPath = path;
}

QString Kryvo::Settings::outputPath() const {
  Q_D(const Settings);

  return d->outputPath;
}

void Kryvo::Settings::lastOpenPath(const QString& path) {
  Q_D(Settings);

  d->lastOpenPath = path;
}

QString Kryvo::Settings::lastOpenPath() const {
  Q_D(const Settings);

  return d->lastOpenPath;
}

QString Kryvo::Settings::styleSheetPath() const {
  Q_D(const Settings);

  return d->styleSheetPath;
}

Kryvo::SettingsPrivate::SettingsPrivate()
  : maximized{false}, keySize{std::size_t{128}}, compressionMode{true},
    containerMode{false}, outputPath{kDefaultPath},
    lastOpenPath{kDefaultPath} {
}

void Kryvo::SettingsPrivate::importSettings() {
#if defined(Q_OS_MACOS)
  const QString settingsPath = qApp->applicationDirPath() %
                               QDir::separator() %
                               QStringLiteral("settings.json");
#else
  const QString settingsPath = QStringLiteral("settings.json");
#endif

  QFile settingsFile{settingsPath};
  const bool fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen) {
    const QByteArray settingsData = settingsFile.readAll();

    const QJsonDocument settingsDoc = QJsonDocument::fromJson(settingsData);
    const QJsonObject settings = settingsDoc.object();

    const QJsonObject positionObject =
      settings[QStringLiteral("position")].toObject();
    position = QPoint{positionObject[QStringLiteral("x")].toInt(100),
                      positionObject[QStringLiteral("y")].toInt(100)};

    maximized = settings[QStringLiteral("maximized")].toBool(false);

    if (!maximized) {
      const QJsonObject sizeObject = settings["size"].toObject();
      size = QSize{sizeObject[QStringLiteral("width")].toInt(800),
                   sizeObject[QStringLiteral("height")].toInt(600)};
    }

    cipher = settings[QStringLiteral("cipher")].toString(QStringLiteral("AES"));

    keySize =
      static_cast<std::size_t>(settings[QStringLiteral("keySize")].toInt(128));

    const QJsonValue modeOfOperationObject =
        settings[QStringLiteral("modeOfOperation")];
    modeOfOperation = modeOfOperationObject.toString(QStringLiteral("GCM"));

    compressionMode = settings[QStringLiteral("compressionMode")].toBool(true);

    containerMode = settings[QStringLiteral("containerMode")].toBool(true);

    outputPath = settings[QStringLiteral("outputPath")].toString(kDefaultPath);

    const QString lastOpen =
      settings[QStringLiteral("lastOpenPath")].toString(kDefaultPath);

    const QFileInfo lastOpenInfo{lastOpen};

    if (lastOpenInfo.exists() && lastOpenInfo.isDir()) {
      lastOpenPath = lastOpenInfo.absolutePath() % QDir::separator();
    }
    else {
      lastOpenPath = kDefaultPath;
    }

    const QJsonValue styleObject = settings[QStringLiteral("styleSheetPath")];
    styleSheetPath = styleObject.toString(QStringLiteral("kryvo.qss"));
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
    styleSheetPath = QStringLiteral("kryvo.qss");
  }
}

void Kryvo::SettingsPrivate::exportSettings() const {
#if defined(Q_OS_MAC)
  const QString settingsPath = QCoreApplication::applicationDirPath() %
                               QDir::separator() %
                               QStringLiteral("settings.json");
#else
  const QString settingsPath = QStringLiteral("settings.json");
#endif

  QSaveFile settingsFile{settingsPath};
  settingsFile.setDirectWriteFallback(true);
  const bool fileOpen = settingsFile.open(QIODevice::WriteOnly);

  if (fileOpen) {
    QJsonObject settings;

    QJsonObject positionObject;
    positionObject[QStringLiteral("x")] = position.x();
    positionObject[QStringLiteral("y")] = position.y();

    settings[QStringLiteral("position")] = positionObject;
    settings[QStringLiteral("maximized")] = maximized;

    if (!maximized) {
      QJsonObject sizeObject;
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
      const QString cleanedPath = QDir::cleanPath(inPath);

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

    const QJsonDocument settingsDoc{settings};
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}
