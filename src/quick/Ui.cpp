#include "Ui.hpp"
#include <QQmlApplicationEngine>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
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

  // Messages to display to user
  const QStringList messages{
    QObject::tr("A password is required to encrypt or decrypt "
                "files. Please enter one to continue."),
    QObject::tr("Encryption/decryption is already in progress. "
                "Please wait until the current operation "
                "finishes.")};

  QQmlApplicationEngine engine;

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

  // Set object name
  this->setObjectName(QStringLiteral("mainWindow"));

  d->engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  // Keep settings object
  settings = s;

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
}

Kryvo::Ui::~Ui() {
//  settings->outputPath(outputFrame->outputPath());
}

void Kryvo::Ui::addFiles( const QStringList& fileNames ) {
  Q_ASSERT(settings);

  // Open a file dialog to get files
//  const QStringList& fileNames =
//    QFileDialog::getOpenFileNames(this, tr("Add Files"),
//                                  settings->lastOpenPath(),
//                                  tr("Any files (*)"));

  if (!fileNames.isEmpty()) { // If files were selected, add them to the model
    for (const QString& fileName : fileNames) {
      const QFileInfo fileInfo(fileName);
//      fileListFrame->addFileToModel(fileInfo.absoluteFilePath());
    }

    // Save this directory for returning to later
    const QFileInfo lastFileInfo(fileNames.last());
    settings->lastOpenPath(lastFileInfo.absolutePath());
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
  Q_ASSERT(settings);

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
                       settings->cipher(),
                       settings->keySize(),
                       settings->modeOfOperation(),
                       settings->compressionMode(),
                       settings->removeIntermediateFiles());
        } else {
          emit decrypt(passphrase, fileList, outputPath,
                       settings->removeIntermediateFiles());
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
  settings->cipher(cipher);
}

void Kryvo::Ui::updateKeySize(const std::size_t keySize) {
  settings->keySize(keySize);
}

void Kryvo::Ui::updateModeOfOperation(const QString& mode) {
  settings->modeOfOperation(mode);
}

void Kryvo::Ui::updateCompressionMode(const bool compress) {
  settings->compressionMode(compress);
}

void Kryvo::Ui::updateRemoveIntermediateFiles(
  const bool removeIntermediate) {
  settings->removeIntermediateFiles(removeIntermediate);
}

void Kryvo::Ui::updateContainerMode(const bool container) {
  settings->containerMode(container);
}
