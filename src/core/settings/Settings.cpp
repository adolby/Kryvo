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

QString addPathSeparator(const QDir& inDir) {
  QString outPath = inDir.absolutePath();

  if (!inDir.isRoot()) {
    outPath = outPath % QStringLiteral("/");
  }

  return outPath;
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
  bool maximized{false};
  QSize size;
  QString cryptoProvider;
  QString compressionFormat;
  QString cipher;
  std::size_t keySize{128};
  QString modeOfOperation;
  bool removeIntermediateFiles{true};
  bool containerMode{false};
  QString outputPath{Kryvo::Constants::kDocumentsPath};
  QString inputPath{Kryvo::Constants::kDocumentsPath};
  QString styleSheetPath;
};

Kryvo::SettingsPrivate::SettingsPrivate() = default;

void Kryvo::SettingsPrivate::importSettings() {
#if defined(Q_OS_MACOS)
  const QString& settingsPath =
    QString(QCoreApplication::instance()->applicationDirPath() %
            QStringLiteral("/") %
            QStringLiteral("settings.json"));
#else
  const QString& settingsPath = QStringLiteral("settings.json");
#endif

  QFile settingsFile(settingsPath);
  const bool fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen) {
    const QByteArray& settingsData = settingsFile.readAll();

    const QJsonDocument& settingsDoc = QJsonDocument::fromJson(settingsData);
    const QJsonObject& settings = settingsDoc.object();

    const QJsonObject& positionObject =
      settings[QStringLiteral("position")].toObject();
    position = QPoint(positionObject[QStringLiteral("x")].toInt(100),
                      positionObject[QStringLiteral("y")].toInt(100));

    maximized = settings[QStringLiteral("maximized")].toBool(false);

    if (!maximized) {
      const QJsonObject& sizeObject =
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

    const QJsonValue& modeOfOperationObject =
      settings[QStringLiteral("modeOfOperation")];
    modeOfOperation = modeOfOperationObject.toString(QStringLiteral("GCM"));

    removeIntermediateFiles =
      settings[QStringLiteral("removeIntermediateFiles")].toBool(true);

    containerMode = settings[QStringLiteral("containerMode")].toBool(true);

    outputPath =
      settings[QStringLiteral("outputPath")].toString(
        Constants::kDocumentsPath);

    inputPath =
      settings[QStringLiteral("inputPath")].toString(
        Constants::kDocumentsPath);

    const QJsonValue& styleObject = settings[QStringLiteral("styleSheetPath")];
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
    outputPath = Constants::kDocumentsPath;
    inputPath = Constants::kDocumentsPath;
    styleSheetPath = QStringLiteral("kryvo.qss");
  }
}

void Kryvo::SettingsPrivate::exportSettings() const {
#if defined(Q_OS_MACOS)
  const QString& settingsPath = QString(QCoreApplication::applicationDirPath() %
                                        QStringLiteral("/") %
                                        QStringLiteral("settings.json"));
#else
  const QString& settingsPath = QStringLiteral("settings.json");
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
      Constants::kDocumentsPath :
      outputPath;
    settings[QStringLiteral("inputPath")] =
      inputPath.isEmpty() ?
      Constants::kDocumentsPath :
      inputPath;
    settings[QStringLiteral("styleSheetPath")] =
      styleSheetPath;

    const QJsonDocument settingsDoc(settings);
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}

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

  d->exportSettings();
}

QPoint Kryvo::Settings::position() const {
  Q_D(const Settings);

  return d->position;
}

void Kryvo::Settings::maximized(const bool maximized) {
  Q_D(Settings);

  d->maximized = maximized;

  d->exportSettings();
}

bool Kryvo::Settings::maximized() const {
  Q_D(const Settings);

  return d->maximized;
}

void Kryvo::Settings::size(const QSize& size) {
  Q_D(Settings);

  d->size = size;

  d->exportSettings();
}

QSize Kryvo::Settings::size() const {
  Q_D(const Settings);

  return d->size;
}

void Kryvo::Settings::cryptoProvider(const QString& provider) {
  Q_D(Settings);

  d->cryptoProvider = provider;

  d->exportSettings();
}

QString Kryvo::Settings::cryptoProvider() const {
  Q_D(const Settings);

  return d->cryptoProvider;
}

void Kryvo::Settings::compressionFormat(const QString& format) {
  Q_D(Settings);

  d->compressionFormat = format;

  d->exportSettings();
}

QString Kryvo::Settings::compressionFormat() const {
  Q_D(const Settings);

  return d->compressionFormat;
}

void Kryvo::Settings::cipher(const QString& cipherName) {
  Q_D(Settings);

  d->cipher = cipherName;

  d->exportSettings();
}

QString Kryvo::Settings::cipher() const {
  Q_D(const Settings);

  return d->cipher;
}

void Kryvo::Settings::keySize(const std::size_t keySize) {
  Q_D(Settings);

  d->keySize = keySize;

  d->exportSettings();
}

std::size_t Kryvo::Settings::keySize() const {
  Q_D(const Settings);

  return d->keySize;
}

void Kryvo::Settings::modeOfOperation(const QString& modeOfOperation) {
  Q_D(Settings);

  d->modeOfOperation = modeOfOperation;

  d->exportSettings();
}

QString Kryvo::Settings::modeOfOperation() const {
  Q_D(const Settings);

  return d->modeOfOperation;
}

void Kryvo::Settings::removeIntermediateFiles(const bool removeIntermediate) {
  Q_D(Settings);

  d->removeIntermediateFiles = removeIntermediate;

  d->exportSettings();
}

bool Kryvo::Settings::removeIntermediateFiles() const {
  Q_D(const Settings);

  return d->removeIntermediateFiles;
}

void Kryvo::Settings::containerMode(const bool container) {
  Q_D(Settings);

  d->containerMode = container;

  d->exportSettings();
}

bool Kryvo::Settings::containerMode() const {
  Q_D(const Settings);

  return d->containerMode;
}

void Kryvo::Settings::outputPath(const QString& path) {
  Q_D(Settings);

  const QDir outputDir(path);

  if (outputDir.exists()) {
    d->outputPath = addPathSeparator(outputDir);
  } else {
    d->outputPath = Constants::kDocumentsPath;
  }

  d->exportSettings();
}

QString Kryvo::Settings::outputPath() const {
  Q_D(const Settings);

  return d->outputPath;
}

void Kryvo::Settings::inputPath(const QString& path) {
  Q_D(Settings);

  const QDir inputDir(path);

  if (inputDir.exists()) {
    d->inputPath = addPathSeparator(inputDir);
  } else {
    d->inputPath = Constants::kDocumentsPath;
  }

  d->exportSettings();
}

QString Kryvo::Settings::inputPath() const {
  Q_D(const Settings);

  return d->inputPath;
}

QString Kryvo::Settings::styleSheetPath() const {
  Q_D(const Settings);

  return d->styleSheetPath;
}
