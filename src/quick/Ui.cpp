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
#include <QStringBuilder>
#include <QString>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>
#endif

namespace Kryvo {

QString qmlUrlToPath(const QUrl& url) {
  QString pathString;

  if (url.isLocalFile()) {
    pathString = QDir::toNativeSeparators(url.toLocalFile());
  }

  return pathString;
}

class UiPrivate {
  Q_DISABLE_COPY(UiPrivate)
  Q_DECLARE_PUBLIC(Ui)

 public:
  explicit UiPrivate(Ui* ui, Settings* s);

  Ui* const q_ptr{nullptr};

  Settings* settings = nullptr;

  // Messages to display to user
  const QStringList errorMessages{
    QObject::tr("A password is required to encrypt or decrypt files. Please "
                "enter one to continue."),
    QObject::tr("First add files to encrypt or decrypt.")};

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

UiPrivate::UiPrivate(Ui* ui, Settings* s)
  : q_ptr(ui), settings(s), mainPageUrl(QStringLiteral("qrc:/qml/main.qml")) {
  Q_Q(Ui);

  statusMessageIterator = statusMessages.cbegin();

  // Request settings updates
  QObject::connect(q,
                   &Ui::requestUpdateCryptoProvider,
                   settings,
                   qOverload<const QString&>(&Settings::cryptoProvider));
  QObject::connect(q, &Ui::requestUpdateKeySize,
                   settings, qOverload<std::size_t>(&Settings::keySize));
  QObject::connect(q,
                   &Ui::requestUpdateModeOfOperation,
                   settings,
                   qOverload<const QString&>(&Settings::modeOfOperation));
  QObject::connect(q,
                   &Ui::requestUpdateCompressionFormat,
                   settings,
                   qOverload<const QString&>(&Settings::compressionFormat));
  QObject::connect(q,
                   &Ui::requestUpdateRemoveIntermediateFiles,
                   settings,
                   qOverload<bool>(&Settings::removeIntermediateFiles));
  QObject::connect(q, &Ui::requestUpdateContainerMode,
                   settings, qOverload<bool>(&Settings::containerMode));
  QObject::connect(q, &Ui::requestUpdateInputPath,
                   settings, qOverload<const QString&>(&Settings::inputPath));
  QObject::connect(q, &Ui::requestUpdateOutputPath,
                   settings, qOverload<const QString&>(&Settings::outputPath));

  // Receive settings updates
  QObject::connect(settings, &Settings::cryptoProviderChanged,
                   q, &Ui::cryptoProviderChanged);
  QObject::connect(settings,
                   &Settings::keySizeChanged,
                   q,
                   [q](std::size_t size) {
                     emit q->keySizeChanged(QString::number(size));
                   });
  QObject::connect(settings, &Settings::compressionFormatChanged,
                   q, &Ui::compressionFormatChanged);
  QObject::connect(settings, &Settings::removeIntermediateFilesChanged,
                   q, &Ui::removeIntermediateFilesChanged);
  QObject::connect(settings, &Settings::containerModeChanged,
                   q, &Ui::containerModeChanged);
  QObject::connect(settings, &Settings::inputPathChanged,
                   q, &Ui::inputPathChanged);
  QObject::connect(settings, &Settings::outputPathChanged,
                   q, &Ui::outputPathChanged);
}

Ui::Ui(Settings* s, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<UiPrivate>(this, s)) {
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

Ui::~Ui() = default;

void Ui::init() {
  Q_D(Ui);

  QQmlContext* const context = d->engine.rootContext();

  if (context) {
    context->setContextProperty(QStringLiteral("ui"), this);
    context->setContextProperty(QStringLiteral("inputFiles"),
                                &d->fileListModel);
  }

  d->engine.load(d->mainPageUrl);
}

QVariantMap Ui::currentPage() const {
  Q_D(const Ui);
  return d->currentPage;
}

QVariantMap Ui::page(int index) const {
  Q_D(const Ui);

  QVariantMap pg;

  const int size = static_cast<int>(d->navigationHistory.size());

  if (index > -1 && index < size) {
    const int indexInt = static_cast<std::size_t>(index);
    pg = d->navigationHistory.at(indexInt);
  }

  return pg;
}

bool Ui::canNavigateBack() const {
  Q_D(const Ui);

  return d->navigationHistory.size() > 1;
}

QUrl Ui::inputPath() const {
  Q_D(const Ui);
  const QDir inputDir(d->settings->inputPath());
  return QUrl::fromLocalFile(inputDir.absolutePath());
}

QUrl Ui::outputPath() const {
  Q_D(const Ui);
  const QDir outputDir(d->settings->outputPath());
  return QUrl::fromLocalFile(outputDir.absolutePath());
}

QString Ui::outputPathString() const {
  Q_D(const Ui);
  const QDir outputDir(d->settings->outputPath());
  return outputDir.absolutePath();
}

QString Ui::password() const {
  Q_D(const Ui);
  return d->password;
}

QString Ui::cryptoProvider() const {
  Q_D(const Ui);
  return d->settings->cryptoProvider();
}

QString Ui::keySize() const {
  Q_D(const Ui);
  return QString::number(d->settings->keySize());
}

QString Ui::modeOfOperation() const {
  Q_D(const Ui);
  return d->settings->modeOfOperation();
}

QString Ui::compressionFormat() const {
  Q_D(const Ui);
  return d->settings->compressionFormat();
}

bool Ui::removeIntermediateFiles() const {
  Q_D(const Ui);
  return d->settings->removeIntermediateFiles();
}

bool Ui::containerMode() const {
  Q_D(const Ui);
  return d->settings->containerMode();
}

QString Ui::statusMessage() const {
  Q_D(const Ui);
  return d->statusMessage;
}

void Ui::changePage(const QString& name) {
  changePage(name, QVariantMap(), 0);
}

void Ui::changePage(const QString& name, const QVariantMap& properties) {
  changePage(name, properties, 0);
}

void Ui::changePage(const QString& name, const QVariantMap& properties,
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

void Ui::navigate(const QString& name, const QVariantMap& properties) {
  Q_D(Ui);

  changePage(name, properties);

  d->navigationHistory.push_back(d->currentPage);
}

void Ui::navigateBack() {
  Q_D(Ui);

  if (canNavigateBack()) {
    d->navigationHistory.pop_back();

    d->currentPage = page(d->navigationHistory.size() - 1);

    emit popPage();
  } else {
    emit quitApp();
  }
}

void Ui::clearNavigationHistory() {
  Q_D(Ui);

  d->navigationHistory.clear();
}

void Ui::addFile(const QUrl& fileUrl) {
  const QList<QUrl> fileUrlList = {
    fileUrl
  };

  addFiles(fileUrlList);
}

void Ui::addFiles(const QList<QUrl>& fileUrls) {
  Q_D(Ui);
  Q_ASSERT(d->settings);

  if (!fileUrls.isEmpty()) { // If files were selected, add them to the model
    for (const QUrl& fileUrl : fileUrls) {
      const QString filePath = qmlUrlToPath(fileUrl);

      const QFileInfo fileInfo(filePath);

      if (fileInfo.exists() && fileInfo.isFile()) {
        d->fileListModel.appendRow(FileItem(fileInfo.absoluteFilePath()));
      }
    }

    // Save the last file directory for returning to later
    const QUrl lastInputUrl = fileUrls.last();
    const QString inPath = qmlUrlToPath(lastInputUrl);

    const QDir inDir(inPath);

    emit requestUpdateInputPath(inDir.absolutePath());
  }
}

void Ui::removeFiles() {
  Q_D(Ui);

  // Signal to cancel current cipher operation if it's in progress
  emit cancel();

  d->fileListModel.clear();
}

void Ui::encryptFiles(const QString& passphrase) {
  processFiles(passphrase, CryptDirection::Encrypt);
}

void Ui::decryptFiles(const QString& passphrase) {
  processFiles(passphrase, CryptDirection::Decrypt);
}

void Ui::processFiles(const QString& passphrase,
                      const CryptDirection cryptDirection) {
  Q_D(const Ui);
  Q_ASSERT(d->settings);

  if (passphrase.isEmpty()) { // Inform user that a passphrase is required
    appendStatusMessage(d->errorMessages[0]);
    return;
  }

  std::vector<QFileInfo> files;

  const int rowCount = d->fileListModel.rowCount();

  for (int row = 0; row < rowCount; ++row) {
    const FileItem item = d->fileListModel.item(row);
    files.push_back(QFileInfo(item.fileName()));
  }

  if (files.empty()) {
    appendStatusMessage(d->errorMessages[1]);
    return;
  }

  if (CryptDirection::Encrypt == cryptDirection) {
    EncryptConfig config;
    config.provider = d->settings->cryptoProvider();
    config.compressionFormat = d->settings->compressionFormat();
    config.passphrase = passphrase;
    config.cipher = d->settings->cipher();
    config.keySize = d->settings->keySize();
    config.modeOfOperation = d->settings->modeOfOperation();
    config.removeIntermediateFiles = d->settings->removeIntermediateFiles();

    emit encrypt(config, files, QDir(d->settings->outputPath()));
  } else if (CryptDirection::Decrypt == cryptDirection) {
    DecryptConfig config;
    config.passphrase = passphrase;
    config.removeIntermediateFiles = d->settings->removeIntermediateFiles();

    emit decrypt(config, files, QDir(d->settings->outputPath()));
  }
}

void Ui::pauseProcessing(const bool pauseStatus) {
  emit pause(pauseStatus);
}

void Ui::updateFileProgress(const QFileInfo& fileInfo,
                            const QString& task,
                            const qint64 progressValue) {
  Q_D(Ui);

  if (task.isEmpty()) {
    // TODO Emit error message indicating internal error
    return;
  }

  d->fileListModel.updateRow(fileInfo.absoluteFilePath(), task, progressValue);
}

void Ui::appendStatusMessage(const QString& message) {
  Q_D(Ui);

  d->statusMessages.push_back(message);
  d->statusMessageIterator = std::prev(d->statusMessages.cend(), 1);

  d->statusMessage = *d->statusMessageIterator;
  emit statusMessageChanged(d->statusMessage);
}

void Ui::appendErrorMessage(const QString& message,
                            const QFileInfo& fileInfo) {
  if (message.contains(QStringLiteral("%1"))) {
    appendStatusMessage(message.arg(fileInfo.absoluteFilePath()));
  } else {
    appendStatusMessage(message);
  }

  updateFileProgress(QFileInfo(fileInfo.absoluteFilePath()), QString(), 0);
}

void Ui::updateCryptoProvider(const QString& provider) {
  emit requestUpdateCryptoProvider(provider);
}

void Ui::updateKeySize(const QString& keySizeString) {
  const std::size_t size = static_cast<std::size_t>(keySizeString.toInt());
  emit requestUpdateKeySize(size);
}

void Ui::updateModeOfOperation(const QString& mode) {
  emit requestUpdateModeOfOperation(mode);
}

void Ui::updateCompressionFormat(const QString& format) {
  emit requestUpdateCompressionFormat(format);
}

void Ui::updateRemoveIntermediateFiles(const bool removeIntermediate) {
  emit requestUpdateRemoveIntermediateFiles(removeIntermediate);
}

void Ui::updateContainerMode(const bool container) {
  emit requestUpdateContainerMode(container);
}

void Ui::updateOutputPath(const QUrl& url) {
  const QString outPath = qmlUrlToPath(url);
  emit requestUpdateOutputPath(outPath);
}

void Ui::navigateMessageLeft() {
  Q_D(Ui);

  if (d->statusMessageIterator != d->statusMessages.cbegin()) {
    d->statusMessageIterator = std::prev(d->statusMessageIterator, 1);

    d->statusMessage = *d->statusMessageIterator;
    emit statusMessageChanged(d->statusMessage);
  }
}

void Ui::navigateMessageRight() {
  Q_D(Ui);

  const auto end = d->statusMessages.cend();

  if (d->statusMessageIterator != end &&
      d->statusMessageIterator != std::prev(end, 1)) {
    d->statusMessageIterator = std::next(d->statusMessageIterator, 1);

    d->statusMessage = *d->statusMessageIterator;
    emit statusMessageChanged(d->statusMessage);
  }
}

void Ui::updatePassword(const QString& password) {
  Q_D(Ui);

  if (d->password != password) {
    d->password = password;
    emit passwordChanged(d->password);
  }
}

} // namespace Kryvo
