#include "settings/Settings.hpp"
#include "FileUtility.h"
#include "Constants.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringBuilder>
#include <QTimer>
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
  Q_DECLARE_PUBLIC(Settings)

 public:
  SettingsPrivate(Settings* settings);

  void loadDefaults();

  /*!
   * \brief importSettings Imports settings from the local settings file
   */
  void importSettings();

  /*!
   * \brief exportSettings Exports settings to the local settings file
   */
  void exportSettings() const;

  Settings* const q_ptr{nullptr};

  // Reference data
  QHash<QString, Plugin> cryptoProviders_;

  // Setting data
  QPoint position_;
  bool maximized_{false};
  QSize size_;
  QString cryptoProvider_;
  QString compressionFormat_;
  QString cipher_;
  std::size_t keySize_{256};
  QString modeOfOperation_;
  bool removeIntermediateFiles_{true};
  bool containerMode_{false};
  QString outputPath_;
  QString inputPath_;
  QString styleSheetPath_;
};

SettingsPrivate::SettingsPrivate(Settings* s)
  : q_ptr(s) {
}

void SettingsPrivate::loadDefaults() {
  position_ = QPoint(100, 100);
  maximized_ = false;
  size_ = QSize(800, 600);
  cryptoProvider_ = QStringLiteral("OpenSsl");
  compressionFormat_ = QStringLiteral("gzip");
  cipher_ = QStringLiteral("AES");
  keySize_ = static_cast<std::size_t>(256);
  modeOfOperation_ = QStringLiteral("GCM");
  removeIntermediateFiles_ = true;
  containerMode_ = false;
  outputPath_ = Constants::documentsPath;
  inputPath_ = Constants::documentsPath;
  styleSheetPath_ =
    QString(QCoreApplication::instance()->applicationDirPath() %
            QStringLiteral("/themes/kryvo.qss"));
}

void SettingsPrivate::importSettings() {
  Q_Q(Settings);

  const QString settingsFilePath =
    QString(QCoreApplication::instance()->applicationDirPath() %
            QStringLiteral("/settings.json"));

  QFileInfo settingsFileInfo(settingsFilePath);
  QByteArray settingsData;

  const int ec = readConfigFile(settingsFileInfo, settingsData);

  if (ec < 1) { // Settings file couldn't be read, so use defaults
    loadDefaults();
    emit q->settingsImported();
    return;
  }

  const QJsonDocument settingsDoc = QJsonDocument::fromJson(settingsData);
  const QJsonObject settings = settingsDoc.object();

  const QJsonObject positionObject =
    settings[QStringLiteral("position")].toObject();
  position_ = QPoint(positionObject[QStringLiteral("x")].toInt(100),
                    positionObject[QStringLiteral("y")].toInt(100));

  maximized_ = settings[QStringLiteral("maximized")].toBool(false);

  const QJsonObject sizeObject =
    settings[QStringLiteral("size")].toObject();
  size_ = QSize(sizeObject[QStringLiteral("width")].toInt(800),
                sizeObject[QStringLiteral("height")].toInt(600));

  const QString provider =
    settings[QStringLiteral("cryptoProvider")].toString();

  if (!cryptoProviders_.contains(provider)) {
    cryptoProvider_ = QStringLiteral("OpenSsl");
  } else {
    cryptoProvider_ = provider;
  }

  const QString compressionFormat =
    settings[QStringLiteral("compressionFormat")].toString();

  if (compressionFormat != QStringLiteral("gzip") &&
      compressionFormat != QStringLiteral("None")) {
    compressionFormat_ = QStringLiteral("gzip");
  } else {
    compressionFormat_ = compressionFormat;
  }

  cipher_ = QStringLiteral("AES");

  keySize_ =
    static_cast<std::size_t>(settings[QStringLiteral("keySize")].toInt(256));

  const QJsonValue modeOfOperationObject =
    settings[QStringLiteral("modeOfOperation")];
  modeOfOperation_ = modeOfOperationObject.toString(QStringLiteral("GCM"));

  removeIntermediateFiles_ =
    settings[QStringLiteral("removeIntermediateFiles")].toBool(true);

  containerMode_ = settings[QStringLiteral("containerMode")].toBool(false);

  outputPath_ =
    settings[QStringLiteral("outputPath")].toString(Constants::documentsPath);

  inputPath_ =
    settings[QStringLiteral("inputPath")].toString(Constants::documentsPath);

  const QJsonValue styleObject = settings[QStringLiteral("styleSheetPath")];

  styleSheetPath_ =
    QString(QCoreApplication::instance()->applicationDirPath() %
            QStringLiteral("/themes/kryvo.qss"));

  emit q->settingsImported();
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
    positionObject[QStringLiteral("x")] = position_.x();
    positionObject[QStringLiteral("y")] = position_.y();

    settings[QStringLiteral("position")] = positionObject;
    settings[QStringLiteral("maximized")] = maximized_;

    QJsonObject sizeObject;
    sizeObject[QStringLiteral("width")] = size_.width();
    sizeObject[QStringLiteral("height")] = size_.height();
    settings[QStringLiteral("size")] = sizeObject;

    settings[QStringLiteral("cryptoProvider")] = cryptoProvider_;
    settings[QStringLiteral("compressionFormat")] = compressionFormat_;
    settings[QStringLiteral("cipher")] = cipher_;
    settings[QStringLiteral("keySize")] = static_cast<int>(keySize_);
    settings[QStringLiteral("modeOfOperation")] = modeOfOperation_;
    settings[QStringLiteral("removeIntermediateFiles")] =
      removeIntermediateFiles_;
    settings[QStringLiteral("containerMode")] = containerMode_;

    settings[QStringLiteral("outputPath")] =
      outputPath_.isEmpty() ?
      Constants::documentsPath :
      outputPath_;
    settings[QStringLiteral("inputPath")] =
      inputPath_.isEmpty() ?
      Constants::documentsPath :
      inputPath_;
    settings[QStringLiteral("styleSheetPath")] = styleSheetPath_;

    const QJsonDocument settingsDoc(settings);
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}

Settings::Settings(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<SettingsPrivate>(this)) {
}

Settings::~Settings() {
  Q_D(Settings);

  d->exportSettings();
}

void Settings::position(const QPoint& position) {
  Q_D(Settings);

  if (position != d->position_) {
    d->position_ = position;
    d->exportSettings();
    emit positionChanged(d->position_);
  }
}

QPoint Settings::position() const {
  Q_D(const Settings);

  return d->position_;
}

void Settings::maximized(const bool maximized) {
  Q_D(Settings);

  if (maximized != d->maximized_) {
    d->maximized_ = maximized;
    d->exportSettings();
    emit maximizedChanged(d->maximized_);
  }
}

bool Settings::maximized() const {
  Q_D(const Settings);

  return d->maximized_;
}

void Settings::size(const QSize& size) {
  Q_D(Settings);

  if (size != d->size_) {
    d->size_ = size;
    d->exportSettings();
    emit sizeChanged(size);
  }
}

QSize Settings::size() const {
  Q_D(const Settings);

  return d->size_;
}

void Settings::cryptoProvider(const QString& provider) {
  Q_D(Settings);

  if (!d->cryptoProviders_.contains(provider)) {
    return;
  }

  if (provider == d->cryptoProvider_) {
    return;
  }

  d->cryptoProvider_ = provider;
  d->exportSettings();
  emit cryptoProviderChanged(d->cryptoProvider_);
}

QString Settings::cryptoProvider() const {
  Q_D(const Settings);

  return d->cryptoProvider_;
}

void Settings::compressionFormat(const QString& format) {
  Q_D(Settings);

  if (format != QStringLiteral("gzip") &&
      format != QStringLiteral("None")) {
    return;
  }

  if (format == d->compressionFormat_) {
    return;
  }

  d->compressionFormat_ = format;
  d->exportSettings();
  emit compressionFormatChanged(d->compressionFormat_);
}

QString Settings::compressionFormat() const {
  Q_D(const Settings);

  return d->compressionFormat_;
}

void Settings::cipher(const QString& cipherName) {
  Q_D(Settings);

  if (cipherName != QStringLiteral("AES")) {
    return;
  }

  if (cipherName == d->cipher_) {
    return;
  }

  d->cipher_ = cipherName;
  d->exportSettings();
  emit cipherChanged(d->cipher_);
}

QString Settings::cipher() const {
  Q_D(const Settings);

  return d->cipher_;
}

void Settings::keySize(const std::size_t keySize) {
  Q_D(Settings);

  if (keySize != d->keySize_) {
    d->keySize_ = keySize;
    d->exportSettings();
    emit keySizeChanged(d->keySize_);
  }
}

std::size_t Settings::keySize() const {
  Q_D(const Settings);

  return d->keySize_;
}

void Settings::modeOfOperation(const QString& modeOfOperation) {
  Q_D(Settings);

  if (modeOfOperation != d->modeOfOperation_) {
    d->modeOfOperation_ = modeOfOperation;
    d->exportSettings();
    emit modeOfOperationChanged(d->modeOfOperation_);
  }
}

QString Settings::modeOfOperation() const {
  Q_D(const Settings);

  return d->modeOfOperation_;
}

void Settings::removeIntermediateFiles(const bool removeIntermediate) {
  Q_D(Settings);

  if (removeIntermediate != d->removeIntermediateFiles_) {
    d->removeIntermediateFiles_ = removeIntermediate;
    d->exportSettings();
    emit removeIntermediateFilesChanged(d->removeIntermediateFiles_);
  }
}

bool Settings::removeIntermediateFiles() const {
  Q_D(const Settings);

  return d->removeIntermediateFiles_;
}

void Settings::containerMode(const bool container) {
  Q_D(Settings);

  if (container != d->containerMode_) {
    d->containerMode_ = container;
    d->exportSettings();
    emit containerModeChanged(d->containerMode_);
  }
}

bool Settings::containerMode() const {
  Q_D(const Settings);

  return d->containerMode_;
}

void Settings::outputPath(const QString& path) {
  Q_D(Settings);

  const QDir outputDir(path);
  QString outputPath = Constants::documentsPath;

  if (outputDir.exists()) {
    outputPath = addPathSeparator(outputDir);
  }

  if (outputPath != d->outputPath_) {
    d->outputPath_ = outputPath;
    d->exportSettings();
    emit outputPathChanged(d->outputPath_);
  }
}

QString Settings::outputPath() const {
  Q_D(const Settings);

  return d->outputPath_;
}

void Settings::inputPath(const QString& path) {
  Q_D(Settings);

  const QDir inputDir(path);
  QString inputPath = Constants::documentsPath;

  if (inputDir.exists()) {
    inputPath = addPathSeparator(inputDir);
  }

  if (inputPath != d->inputPath_) {
    d->inputPath_ = inputPath;
    d->exportSettings();
    emit inputPathChanged(d->inputPath_);
  }
}

QString Settings::inputPath() const {
  Q_D(const Settings);

  return d->inputPath_;
}

QString Settings::styleSheetPath() const {
  Q_D(const Settings);

  return d->styleSheetPath_;
}

void Settings::cryptoProvidersLoaded(const QHash<QString, Plugin>& providers) {
  Q_D(Settings);

  d->cryptoProviders_ = providers;

  d->importSettings();
}

} // namespace Kryvo
