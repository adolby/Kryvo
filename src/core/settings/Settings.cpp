#include "settings/Settings.hpp"
#include "Constants.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringBuilder>
#include <QCoreApplication>

namespace Kryvo {

QString addPathSeparator(const QDir& inDir) {
  QString outPath = inDir.absolutePath();

  if (!inDir.isRoot()) {
    outPath = outPath % QStringLiteral("/");
  }

  return outPath;
}

class SettingsPrivate {
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
  bool maximized{false};
  QSize size;
  QString cryptoProvider;
  QString compressionFormat;
  QString cipher;
  std::size_t keySize{128};
  QString modeOfOperation;
  bool removeIntermediateFiles{true};
  bool containerMode{false};
  QString outputPath{Constants::documentsPath};
  QString inputPath{Constants::documentsPath};
  QString styleSheetPath;
};

SettingsPrivate::SettingsPrivate() = default;

void SettingsPrivate::importSettings() {
#if defined(Q_OS_MACOS)
  const QString settingsPath =
    QString(QCoreApplication::instance()->applicationDirPath() %
            QStringLiteral("/") %
            QStringLiteral("settings.json"));
#else
  const QString settingsPath = QStringLiteral("settings.json");
#endif

  QFile settingsFile(settingsPath);
  const bool fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen) {
    const QByteArray settingsData = settingsFile.readAll();

    const QJsonDocument settingsDoc = QJsonDocument::fromJson(settingsData);
    const QJsonObject settings = settingsDoc.object();

    const QJsonObject positionObject =
      settings[QStringLiteral("position")].toObject();
    position = QPoint(positionObject[QStringLiteral("x")].toInt(100),
                      positionObject[QStringLiteral("y")].toInt(100));

    maximized = settings[QStringLiteral("maximized")].toBool(false);

    if (!maximized) {
      const QJsonObject sizeObject =
        settings[QStringLiteral("size")].toObject();
      size = QSize(sizeObject[QStringLiteral("width")].toInt(800),
                   sizeObject[QStringLiteral("height")].toInt(600));
    }

    cryptoProvider =
      settings[QStringLiteral("cryptoProvider")].toString(QStringLiteral("Botan"));

    compressionFormat =
      settings[QStringLiteral("compressionFormat")].toString(QStringLiteral("gzip"));

    cipher = settings[QStringLiteral("cipher")].toString(QStringLiteral("AES"));

    keySize =
      static_cast<std::size_t>(settings[QStringLiteral("keySize")].toInt(128));

    const QJsonValue modeOfOperationObject =
      settings[QStringLiteral("modeOfOperation")];
    modeOfOperation = modeOfOperationObject.toString(QStringLiteral("GCM"));

    removeIntermediateFiles =
      settings[QStringLiteral("removeIntermediateFiles")].toBool(true);

    containerMode = settings[QStringLiteral("containerMode")].toBool(true);

    outputPath =
      settings[QStringLiteral("outputPath")].toString(Constants::documentsPath);

    inputPath =
      settings[QStringLiteral("inputPath")].toString(Constants::documentsPath);

    const QJsonValue styleObject = settings[QStringLiteral("styleSheetPath")];
    styleSheetPath = styleObject.toString(QStringLiteral("kryvo.qss"));
  } else { // Settings file couldn't be opened, so use defaults
    position = QPoint(100, 100);
    maximized = false;
    size = QSize(800, 600);
    cryptoProvider = QStringLiteral("Botan");
    compressionFormat = QStringLiteral("gzip");
    cipher = QStringLiteral("AES");
    keySize = std::size_t(128);
    modeOfOperation = QStringLiteral("GCM");
    removeIntermediateFiles = true;
    containerMode = true;
    outputPath = Constants::documentsPath;
    inputPath = Constants::documentsPath;
    styleSheetPath = QStringLiteral("kryvo.qss");
  }
}

void SettingsPrivate::exportSettings() const {
#if defined(Q_OS_MACOS)
  const QString settingsPath = QString(QCoreApplication::applicationDirPath() %
                                       QStringLiteral("/") %
                                       QStringLiteral("settings.json"));
#else
  const QString settingsPath = QStringLiteral("settings.json");
#endif

  QSaveFile settingsFile(settingsPath);
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

    settings[QStringLiteral("cryptoProvider")] = cryptoProvider;
    settings[QStringLiteral("compressionFormat")] = compressionFormat;
    settings[QStringLiteral("cipher")] = cipher;
    settings[QStringLiteral("keySize")] = static_cast<int>(keySize);
    settings[QStringLiteral("modeOfOperation")] = modeOfOperation;
    settings[QStringLiteral("removeIntermediateFiles")] =
      removeIntermediateFiles;
    settings[QStringLiteral("containerMode")] = containerMode;

    settings[QStringLiteral("outputPath")] =
      outputPath.isEmpty() ?
      Constants::documentsPath :
      outputPath;
    settings[QStringLiteral("inputPath")] =
      inputPath.isEmpty() ?
      Constants::documentsPath :
      inputPath;
    settings[QStringLiteral("styleSheetPath")] =
      styleSheetPath;

    const QJsonDocument settingsDoc(settings);
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}

Settings::Settings()
  : d_ptr{std::make_unique<SettingsPrivate>()} {
  Q_D(Settings);

  d->importSettings();
}

Settings::~Settings() {
  Q_D(Settings);

  d->exportSettings();
}

void Settings::position(const QPoint& position) {
  Q_D(Settings);

  d->position = position;

  d->exportSettings();
}

QPoint Settings::position() const {
  Q_D(const Settings);

  return d->position;
}

void Settings::maximized(const bool maximized) {
  Q_D(Settings);

  d->maximized = maximized;

  d->exportSettings();
}

bool Settings::maximized() const {
  Q_D(const Settings);

  return d->maximized;
}

void Settings::size(const QSize& size) {
  Q_D(Settings);

  d->size = size;

  d->exportSettings();
}

QSize Settings::size() const {
  Q_D(const Settings);

  return d->size;
}

void Settings::cryptoProvider(const QString& provider) {
  Q_D(Settings);

  d->cryptoProvider = provider;

  d->exportSettings();
}

QString Settings::cryptoProvider() const {
  Q_D(const Settings);

  return d->cryptoProvider;
}

void Settings::compressionFormat(const QString& format) {
  Q_D(Settings);

  d->compressionFormat = format;

  d->exportSettings();
}

QString Settings::compressionFormat() const {
  Q_D(const Settings);

  return d->compressionFormat;
}

void Settings::cipher(const QString& cipherName) {
  Q_D(Settings);

  d->cipher = cipherName;

  d->exportSettings();
}

QString Settings::cipher() const {
  Q_D(const Settings);

  return d->cipher;
}

void Settings::keySize(const std::size_t keySize) {
  Q_D(Settings);

  d->keySize = keySize;

  d->exportSettings();
}

std::size_t Settings::keySize() const {
  Q_D(const Settings);

  return d->keySize;
}

void Settings::modeOfOperation(const QString& modeOfOperation) {
  Q_D(Settings);

  d->modeOfOperation = modeOfOperation;

  d->exportSettings();
}

QString Settings::modeOfOperation() const {
  Q_D(const Settings);

  return d->modeOfOperation;
}

void Settings::removeIntermediateFiles(const bool removeIntermediate) {
  Q_D(Settings);

  d->removeIntermediateFiles = removeIntermediate;

  d->exportSettings();
}

bool Settings::removeIntermediateFiles() const {
  Q_D(const Settings);

  return d->removeIntermediateFiles;
}

void Settings::containerMode(const bool container) {
  Q_D(Settings);

  d->containerMode = container;

  d->exportSettings();
}

bool Settings::containerMode() const {
  Q_D(const Settings);

  return d->containerMode;
}

void Settings::outputPath(const QString& path) {
  Q_D(Settings);

  const QDir outputDir(path);

  if (outputDir.exists()) {
    d->outputPath = addPathSeparator(outputDir);
  } else {
    d->outputPath = Constants::documentsPath;
  }

  d->exportSettings();
}

QString Settings::outputPath() const {
  Q_D(const Settings);

  return d->outputPath;
}

void Settings::inputPath(const QString& path) {
  Q_D(Settings);

  const QDir inputDir(path);

  if (inputDir.exists()) {
    d->inputPath = addPathSeparator(inputDir);
  } else {
    d->inputPath = Constants::documentsPath;
  }

  d->exportSettings();
}

QString Settings::inputPath() const {
  Q_D(const Settings);

  return d->inputPath;
}

QString Settings::styleSheetPath() const {
  Q_D(const Settings);

  return d->styleSheetPath;
}

} // namespace Kryvo
