#include "Ui.hpp"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QTimer>
#include <QSize>
#include <QStringList>
#include <QList>
#include <QStringRef>
#include <QStringBuilder>
#include <QString>

class Kryvo::UiPrivate {
  Q_DISABLE_COPY(UiPrivate)

 public:
  UiPrivate();

  /*!
   * \brief busy Sets the busy status received from the cipher operation.
   * \param busy Boolean representing the busy status.
   */
  void busy(bool busy);

  /*!
   * \brief isBusy Returns the busy status received from the cipher operation.
   * \return Boolean representing the busy status.
   */
  bool isBusy() const;

  Settings* settings = nullptr;

  // Messages to display to user
  const QStringList messages{
    QObject::tr("A password is required to encrypt or decrypt "
                "files. Please enter one to continue."),
    QObject::tr("Encryption/decryption is already in progress. "
                "Please wait until the current operation "
                "finishes.")};

  QQmlApplicationEngine engine;

  QVariantMap currentPage;
  std::vector<QVariantMap> navigationHistory;

 private:
  // The busy status, when set to true, indicates that the cryptography object
  // is currently executing a cipher operation. The status allows the GUI to
  // decide whether to send new encryption/decryption requests.
  bool busyStatus{false};
};

Kryvo::UiPrivate::UiPrivate() = default;

void Kryvo::UiPrivate::busy(const bool busy) {
  busyStatus = busy;
}

bool Kryvo::UiPrivate::isBusy() const {
  return busyStatus;
}

Kryvo::Ui::Ui(Settings* s, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<UiPrivate>()) {
  Q_D(Ui);

  // Keep settings object
  d->settings = s;

  // Header button connections
//  connect(headerFrame, &HeaderFrame::pause,
//          this, &Ui::pause);
//  connect(headerFrame, &HeaderFrame::addFiles,
//          this, &Ui::addFiles);
//  connect(headerFrame, &HeaderFrame::removeFiles,
//          this, &Ui::removeFiles);

  // Settings frame connections
//  connect(settingsFrame, &SettingsFrame::updateCipher,
//          this, &Ui::updateCipher);

//  connect(settingsFrame, &SettingsFrame::updateKeySize,
//          this, &Ui::updateKeySize);

//  connect(settingsFrame, &SettingsFrame::updateModeOfOperation,
//          this, &Ui::updateModeOfOperation);

//  connect(settingsFrame, &SettingsFrame::updateCompressionMode,
//          this, &Ui::updateCompressionMode);

//  connect(settingsFrame, &SettingsFrame::updateRemoveIntermediateFiles,
//          this, &Ui::updateRemoveIntermediateFiles);

//  connect(settingsFrame, &SettingsFrame::updateContainerMode,
//          this, &Ui::updateContainerMode);

  // Forwarded connections
//  connect(fileListFrame, &FileListFrame::stopFile,
//          this, &Ui::stopFile, Qt::DirectConnection);
//  connect(controlButtonFrame, &ControlButtonFrame::processFiles,
//          this, &Ui::processFiles);

  QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

Kryvo::Ui::~Ui() {
  Q_D(Ui);
//  d->settings->outputPath(outputFrame->outputPath());
}

void Kryvo::Ui::init() {
  Q_D(Ui);

  QQmlContext* context = d->engine.rootContext();

  if (context) {
    context->setContextProperty(QStringLiteral("ui"), this);
  }

  d->engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
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
    pg = d->navigationHistory.at(index);
  }

  return pg;
}

bool Kryvo::Ui::canNavigateBack() const {
  Q_D(const Ui);

  return d->navigationHistory.size() > 1;
}

void Kryvo::Ui::changePage(const QString& name, const QVariantMap& properties,
                           const int delayInMSecs) {
  Q_D(Ui);

  const auto changeThePage = [this, name, properties]() {
    Q_D(Ui);

    QVariantMap page;
    page.insert(QStringLiteral("name"), name);

    if (!properties.isEmpty()) {
        page.insert(QStringLiteral("properties"), properties);
    }

    d->currentPage = page;

    emit pageChanged(d->currentPage);
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

    emit pageChanged(d->currentPage);
  } else {
//    goToAndroidHomeScreen();
  }
}

void Kryvo::Ui::clearNavigationHistory() {
  Q_D(Ui);

  d->navigationHistory.clear();
}

void Kryvo::Ui::addFiles(const QStringList& fileNames) {
  Q_D(Ui);

  Q_ASSERT(d->settings);

  // Open a file dialog to get files
//  const QStringList& fileNames =
//    QFileDialog::getOpenFileNames(this, tr("Add Files"),
//                                  d->settings->lastOpenPath(),
//                                  tr("Any files (*)"));

  if (!fileNames.isEmpty()) { // If files were selected, add them to the model
    for (const QString& fileName : fileNames) {
      const QFileInfo fileInfo(fileName);
//      fileListFrame->addFileToModel(fileInfo.absoluteFilePath());
    }

    // Save this directory for returning to later
    const QFileInfo lastFileInfo(fileNames.last());
    d->settings->lastOpenPath(lastFileInfo.absolutePath());
  }
}

void Kryvo::Ui::removeFiles() {
  // Signal to abort current cipher operation if it's in progress
  emit abort();

//  fileListFrame->clear();
}

void Kryvo::Ui::processFiles(const QString& outputPath,
                             const QString& passphrase,
                             const bool cryptDirection) {
  Q_D(Ui);

  Q_ASSERT(d->settings);

  if (!d->isBusy()) {
    if (!passphrase.isEmpty()) {
        QStringList fileList;

//        for (int row = 0; row < rowCount; ++row) {
//          auto item = fileListFrame->item(row);
//          fileList.append(item->data().toString());
//        }

        if (cryptDirection) {
          emit encrypt(passphrase,
                       fileList,
                       outputPath,
                       d->settings->cipher(),
                       d->settings->keySize(),
                       d->settings->modeOfOperation(),
                       d->settings->compressionMode(),
                       d->settings->removeIntermediateFiles());
        } else {
          emit decrypt(passphrase, fileList, outputPath,
                       d->settings->removeIntermediateFiles());
        }
    } else { // Inform user that a password is required to encrypt or decrypt
      updateStatusMessage(d->messages[0]);
    }
  } else {
    // Inform user that encryption/decryption is already in progress
    updateStatusMessage(d->messages[1]);
  }
}

void Kryvo::Ui::updateFileProgress(const QString& filePath,
                                   const QString& task,
                                   const qint64 progressValue) {
//  fileListFrame->updateProgress(filePath, task, progressValue);
}

void Kryvo::Ui::updateStatusMessage(const QString& message) {
//  messageFrame->appendText(message);
}

void Kryvo::Ui::updateError(const QString& message,
                            const QString& fileName) {
  if (message.contains(QStringLiteral("%1"))) {
    updateStatusMessage(message.arg(fileName));
  } else {
    updateStatusMessage(message);
  }

  updateFileProgress(fileName, QString(), 0);
}

void Kryvo::Ui::updateBusyStatus(const bool busy) {
  Q_D(Ui);
  d->busy(busy);
}

void Kryvo::Ui::updateCipher(const QString& cipher) {
  Q_D(Ui);
  d->settings->cipher(cipher);
}

void Kryvo::Ui::updateKeySize(const std::size_t keySize) {
  Q_D(Ui);
  d->settings->keySize(keySize);
}

void Kryvo::Ui::updateModeOfOperation(const QString& mode) {
  Q_D(Ui);
  d->settings->modeOfOperation(mode);
}

void Kryvo::Ui::updateCompressionMode(const bool compress) {
  Q_D(Ui);
  d->settings->compressionMode(compress);
}

void Kryvo::Ui::updateRemoveIntermediateFiles(
  const bool removeIntermediate) {
  Q_D(Ui);
  d->settings->removeIntermediateFiles(removeIntermediate);
}

void Kryvo::Ui::updateContainerMode(const bool container) {
  Q_D(Ui);
  d->settings->containerMode(container);
}
