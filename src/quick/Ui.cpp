#include "Ui.hpp"
#include "models/FileListModel.hpp"
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QSize>
#include <QStringList>
#include <QList>
#include <QStringRef>
#include <QStringBuilder>
#include <QString>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>
#endif

QString qmlUrlToPath(const QUrl& url) {
  QString pathString;

  if (url.isLocalFile()) {
    pathString = QDir::toNativeSeparators(url.toLocalFile());
  }

  return pathString;
}

class Kryvo::UiPrivate {
  Q_DISABLE_COPY(UiPrivate)

 public:
  UiPrivate();

  Settings* settings = nullptr;

  // Messages to display to user
  const QStringList errorMessages{
    QObject::tr("A password is required to encrypt or decrypt "
                "files. Please enter one to continue.")};

  QQmlApplicationEngine engine;
  QUrl mainPageUrl;

  FileListModel fileListModel;

  QString statusMessage;
  std::vector<QString> statusMessages;
  std::vector<QString>::const_iterator statusMessageIterator;

  QVariantMap currentPage;
  std::vector<QVariantMap> navigationHistory;

  QString password;
};

Kryvo::UiPrivate::UiPrivate()
  : mainPageUrl(QStringLiteral("qrc:/qml/main.qml")) {
}

Kryvo::Ui::Ui(Settings* s, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<UiPrivate>()) {
  // Keep settings object
  d_ptr->settings = s;

  connect(&d_ptr->engine,
          &QQmlApplicationEngine::objectCreated,
          this,
          [this](QObject* obj, const QUrl& objUrl) {
            Q_D(Ui);

            if (!obj && objUrl == d->mainPageUrl) {
              QCoreApplication::exit(-1);
            }

#if defined(Q_OS_ANDROID)
            QtAndroid::hideSplashScreen(100);
#endif
          },
          Qt::QueuedConnection);

  QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

Kryvo::Ui::~Ui() = default;

void Kryvo::Ui::init() {
  Q_D(Ui);

  QQmlContext* const context = d->engine.rootContext();

  if (context) {
    context->setContextProperty(QStringLiteral("ui"), this);
    context->setContextProperty(QStringLiteral("inputFiles"),
                                &d->fileListModel);
  }

  d->engine.load(d->mainPageUrl);
}

QVariantMap Kryvo::Ui::currentPage() const {
    Q_D(const Ui);
    return d->currentPage;
}

QVariantMap Kryvo::Ui::page(int index) const {
  Q_D(const Ui);

  QVariantMap pg;

  const int size = static_cast<int>(d->navigationHistory.size());

  if (index > -1 && index < size) {
    const int indexInt = static_cast<std::size_t>(index);
    pg = d->navigationHistory.at(indexInt);
  }

  return pg;
}

bool Kryvo::Ui::canNavigateBack() const {
  Q_D(const Ui);

  return d->navigationHistory.size() > 1;
}

QUrl Kryvo::Ui::inputPath() const {
  Q_D(const Ui);
  const QDir& inputDir = d->settings->inputPath();
  return QUrl::fromLocalFile(inputDir.absolutePath());
}

QUrl Kryvo::Ui::outputPath() const {
  Q_D(const Ui);
  const QDir& outputDir = d->settings->outputPath();
  return QUrl::fromLocalFile(outputDir.absolutePath());
}

QString Kryvo::Ui::outputPathString() const {
  Q_D(const Ui);
  const QDir& outputDir = d->settings->outputPath();
  return outputDir.absolutePath();
}

QString Kryvo::Ui::password() const {
  Q_D(const Ui);
  return d->password;
}

QString Kryvo::Ui::cipher() const {
  Q_D(const Ui);
  return d->settings->cipher();
}

QString Kryvo::Ui::keySize() const {
  Q_D(const Ui);
  return QString::number(d->settings->keySize());
}

QString Kryvo::Ui::modeOfOperation() const {
  Q_D(const Ui);
  return d->settings->modeOfOperation();
}

QString Kryvo::Ui::compressionFormat() const {
  Q_D(const Ui);
  return d->settings->compressionFormat();
}

bool Kryvo::Ui::removeIntermediateFiles() const {
  Q_D(const Ui);
  return d->settings->removeIntermediateFiles();
}

bool Kryvo::Ui::containerMode() const {
  Q_D(const Ui);
  return d->settings->containerMode();
}

QString Kryvo::Ui::statusMessage() const {
  Q_D(const Ui);
  return d->statusMessage;
}

void Kryvo::Ui::changePage(const QString& name) {
  changePage(name, QVariantMap(), 0);
}

void Kryvo::Ui::changePage(const QString& name, const QVariantMap& properties) {
  changePage(name, properties, 0);
}

void Kryvo::Ui::changePage(const QString& name, const QVariantMap& properties,
                           const int delayInMSecs) {
  const auto changeThePage = [this, name, properties]() {
    Q_D(Ui);

    QVariantMap page;
    page.insert(QStringLiteral("name"), name);

    if (!properties.isEmpty()) {
        page.insert(QStringLiteral("properties"), properties);
    }

    d->currentPage = page;

    emit pushPage(d->currentPage);
  };

  if (delayInMSecs > 0) {
    QTimer::singleShot(delayInMSecs, this, changeThePage);
  } else {
    changeThePage();
  }
}

void Kryvo::Ui::navigate(const QString& name, const QVariantMap& properties) {
  Q_D(Ui);

  changePage(name, properties);

  d->navigationHistory.push_back(d->currentPage);
}

void Kryvo::Ui::navigateBack() {
  Q_D(Ui);

  if (canNavigateBack()) {
    d->navigationHistory.pop_back();

    d->currentPage = page(d->navigationHistory.size() - 1);

    emit popPage();
  } else {
    emit quitApp();
  }
}

void Kryvo::Ui::clearNavigationHistory() {
  Q_D(Ui);

  d->navigationHistory.clear();
}

void Kryvo::Ui::addFile(const QUrl& fileUrl) {
    const QList<QUrl> fileUrlList = {
      fileUrl
    };

  addFiles(fileUrlList);
}

void Kryvo::Ui::addFiles(const QList<QUrl>& fileUrls) {
  Q_D(Ui);

  Q_ASSERT(d->settings);

  if (!fileUrls.isEmpty()) { // If files were selected, add them to the model
    for (const QUrl& fileUrl : fileUrls) {
      const QString& filePath = qmlUrlToPath(fileUrl);

      const QFileInfo fileInfo(filePath);

      if (fileInfo.exists() && fileInfo.isFile()) {
        d->fileListModel.appendRow(FileItem(fileInfo.absoluteFilePath()));
      } else {
        // TODO Emit error
      }
    }

    // Save the last file directory for returning to later
    const QUrl& lastInputUrl = fileUrls.last();
    const QString& inputPath = qmlUrlToPath(lastInputUrl);

    const QDir inputDir(inputPath);

    if (inputDir != QDir(d->settings->inputPath())) {
      d->settings->inputPath(inputDir.absolutePath());

      emit inputPathChanged(lastInputUrl);
    }
  }
}

void Kryvo::Ui::removeFiles() {
  Q_D(Ui);

  // Signal to abort current cipher operation if it's in progress
  emit abort();

  d->fileListModel.clear();
}

void Kryvo::Ui::encryptFiles(const QString& passphrase) {
  processFiles(passphrase, Kryvo::CryptDirection::Encrypt);
}

void Kryvo::Ui::decryptFiles(const QString& passphrase) {
  processFiles(passphrase, Kryvo::CryptDirection::Decrypt);
}

void Kryvo::Ui::processFiles(const QString& passphrase,
                             const Kryvo::CryptDirection cryptDirection) {
  Q_D(Ui);

  Q_ASSERT(d->settings);

  if (!passphrase.isEmpty()) {
    std::vector<QFileInfo> files;

    const int rowCount = d->fileListModel.rowCount();

    for (int row = 0; row < rowCount; ++row) {
      const FileItem& item = d->fileListModel.item(row);
      files.push_back(QFileInfo(item.fileName()));
    }

    if (files.empty()) {
      // TODO Inform user that input files are required to encrypt or
      // decrypt
      return;
    }

    if (Kryvo::CryptDirection::Encrypt == cryptDirection) {
      emit encrypt(d->settings->cryptoProvider(),
                   d->settings->compressionFormat(),
                   passphrase,
                   files,
                   d->settings->outputPath(),
                   d->settings->cipher(),
                   d->settings->keySize(),
                   d->settings->modeOfOperation(),
                   d->settings->removeIntermediateFiles());
    } else if (Kryvo::CryptDirection::Decrypt == cryptDirection) {
      emit decrypt(passphrase, files, d->settings->outputPath(),
                   d->settings->removeIntermediateFiles());
    }
  } else { // Inform user that a password is required to encrypt or decrypt
    appendStatusMessage(d->errorMessages.at(0));
  }
}

void Kryvo::Ui::pauseProcessing(const bool pauseStatus) {
  emit pause(pauseStatus);
}

void Kryvo::Ui::updateFileProgress(const QFileInfo& fileInfo,
                                   const QString& task,
                                   const qint64 progressValue) {
  Q_D(Ui);

  if (task.isEmpty()) {
    // TODO Emit error message indicating internal error
    return;
  }

  d->fileListModel.updateRow(fileInfo.absoluteFilePath(), task, progressValue);
}

void Kryvo::Ui::appendStatusMessage(const QString& message) {
  Q_D(Ui);

  d->statusMessages.push_back(message);
  d->statusMessageIterator = std::prev(d->statusMessages.end(), 1);

  d->statusMessage = *d->statusMessageIterator;
  emit statusMessageChanged(d->statusMessage);
}

void Kryvo::Ui::appendErrorMessage(const QString& message,
                                   const QFileInfo& fileInfo) {
  if (message.contains(QStringLiteral("%1"))) {
    appendStatusMessage(message.arg(fileInfo.absoluteFilePath()));
  } else {
    appendStatusMessage(message);
  }

  updateFileProgress(fileInfo.absoluteFilePath(), QString(), 0);
}

void Kryvo::Ui::updateCipher(const QString& cipher) {
  Q_D(Ui);

  d->settings->cipher(cipher);
}

void Kryvo::Ui::updateKeySize(const QString& keySizeString) {
  Q_D(Ui);

  const std::size_t keySize = static_cast<std::size_t>(keySizeString.toInt());

  if (keySize != d->settings->keySize()) {
    d->settings->keySize(keySize);

    const QString& updatedKeySizeString =
      QString::number(d->settings->keySize());

    emit keySizeChanged(updatedKeySizeString);
  }
}

void Kryvo::Ui::updateModeOfOperation(const QString& mode) {
  Q_D(Ui);

  if (mode != d->settings->modeOfOperation()) {
    d->settings->modeOfOperation(mode);
    emit modeOfOperationChanged(d->settings->modeOfOperation());
  }
}

void Kryvo::Ui::updateCompressionFormat(const QString& format) {
  Q_D(Ui);

  if (format != d->settings->compressionFormat()) {
    d->settings->compressionFormat(format);
    emit compressionFormatChanged(d->settings->compressionFormat());
  }
}

void Kryvo::Ui::updateRemoveIntermediateFiles(const bool removeIntermediate) {
  Q_D(Ui);

  if (removeIntermediate != d->settings->removeIntermediateFiles()) {
    d->settings->removeIntermediateFiles(removeIntermediate);
    emit removeIntermediateFilesChanged(d->settings->removeIntermediateFiles());
  }
}

void Kryvo::Ui::updateContainerMode(const bool container) {
  Q_D(Ui);

  if (container != d->settings->containerMode()) {
    d->settings->containerMode(container);
    emit containerModeChanged(d->settings->containerMode());
  }
}

void Kryvo::Ui::updateOutputPath(const QUrl& url) {
  Q_D(Ui);

  const QString& outputPath = qmlUrlToPath(url);

  if (outputPath != d->settings->outputPath()) {
    d->settings->outputPath(outputPath);
    emit outputPathChanged(url);
  }
}

void Kryvo::Ui::navigateMessageLeft() {
  Q_D(Ui);

  if (d->statusMessageIterator != d->statusMessages.begin()) {
    d->statusMessageIterator = std::prev(d->statusMessageIterator, 1);

    d->statusMessage = *d->statusMessageIterator;
    emit statusMessageChanged(d->statusMessage);
  }
}

void Kryvo::Ui::navigateMessageRight() {
  Q_D(Ui);

  const auto end = d->statusMessages.end();

  if (d->statusMessageIterator != std::prev(end, 1) &&
      d->statusMessageIterator != end) {
    d->statusMessageIterator = std::next(d->statusMessageIterator, 1);

    d->statusMessage = *d->statusMessageIterator;
    emit statusMessageChanged(d->statusMessage);
  }
}

void Kryvo::Ui::updatePassword(const QString& password) {
  Q_D(Ui);

  if (d->password != password) {
    d->password = password;
    emit passwordChanged(d->password);
  }
}
