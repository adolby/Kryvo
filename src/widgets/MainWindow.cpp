#include "MainWindow.hpp"
#include "SlidingStackedWidget.hpp"
#include "FileUtility.h"
#include <QFrame>
#include <QFileDialog>
#include <QBoxLayout>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QSize>
#include <QStringList>
#include <QList>
#include <QStringBuilder>
#include <QString>

namespace Kryvo {

class MainWindowPrivate {
  Q_DISABLE_COPY(MainWindowPrivate)
  Q_DECLARE_PUBLIC(MainWindow)

 public:
  MainWindowPrivate(MainWindow* mw);

  MainWindow* const q_ptr{nullptr};

  // Messages to display to user
  const QStringList messages{
    QObject::tr("A password is required to encrypt or decrypt "
                "files. Please enter one to continue.")};
};

MainWindowPrivate::MainWindowPrivate(MainWindow* mw)
  : q_ptr(mw) {
}

MainWindow::MainWindow(Settings* s, QWidget* parent)
  : QMainWindow(parent), d_ptr(std::make_unique<MainWindowPrivate>(this)),
    settings(s) {
  // Set object name
  this->setObjectName(QStringLiteral("mainWindow"));

  // Title
  this->setWindowTitle(tr("Kryvo"));

  auto slidingStackedWidget = new SlidingStackedWidget(this);

  auto contentFrame = new QFrame(slidingStackedWidget);
  contentFrame->setObjectName(QStringLiteral("contentFrame"));

  // Header label and operation button frame
  headerFrame = new HeaderFrame(contentFrame);
  headerFrame->setObjectName(QStringLiteral("headerFrame"));
  headerFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  // File list frame
  fileListFrame = new FileListFrame(contentFrame);
  fileListFrame->setObjectName(QStringLiteral("fileListFrame"));
  fileListFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Message text edit display frame
  messageFrame = new MessageFrame(contentFrame);
  messageFrame->setObjectName(QStringLiteral("messageFrame"));
  messageFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  messageFrame->setMinimumHeight(36);

  // Archive file name frame
  outputFrame = new OutputFrame(contentFrame);
  outputFrame->setObjectName(QStringLiteral("outputFrame"));

  // Password entry frame
  passwordFrame = new PasswordFrame(contentFrame);
  passwordFrame->setObjectName(QStringLiteral("passwordFrame"));

  // Encrypt and decrypt control button frame
  controlButtonFrame = new ControlButtonFrame(contentFrame);
  controlButtonFrame->setObjectName(QStringLiteral("controlButtonFrame"));

  contentLayout = new QVBoxLayout(contentFrame);
  contentLayout->addWidget(headerFrame);
  contentLayout->addWidget(fileListFrame);
  contentLayout->addWidget(messageFrame);
  contentLayout->addWidget(outputFrame);
  contentLayout->addWidget(passwordFrame);
  contentLayout->addWidget(controlButtonFrame);
  contentLayout->setSpacing(0);

  slidingStackedWidget->addWidget(contentFrame);

  settingsFrame = new SettingsFrame(slidingStackedWidget);
  settingsFrame->setObjectName(QStringLiteral("settingsFrame"));

  slidingStackedWidget->addWidget(settingsFrame);

  this->setCentralWidget(slidingStackedWidget);

  // Sliding stacked widget connections
  connect(headerFrame, &HeaderFrame::switchFrame,
          slidingStackedWidget, &SlidingStackedWidget::slideInNext);
  connect(settingsFrame, &SettingsFrame::switchFrame,
          slidingStackedWidget, &SlidingStackedWidget::slideInPrev);

  // Header button connections
  connect(headerFrame, &HeaderFrame::pause,
          this, &MainWindow::pause);
  connect(headerFrame, &HeaderFrame::addFiles,
          this, &MainWindow::addFiles);
  connect(headerFrame, &HeaderFrame::removeFiles,
          this, &MainWindow::removeFiles);

  // Request setting update
  connect(settingsFrame, &SettingsFrame::requestUpdateCryptoProvider,
          settings, qOverload<const QString&>(&Settings::cryptoProvider));
  connect(settingsFrame, &SettingsFrame::requestUpdateKeySize,
          settings, qOverload<std::size_t>(&Settings::keySize));
  connect(settingsFrame, &SettingsFrame::requestUpdateCompressionFormat,
          settings, qOverload<const QString&>(&Settings::compressionFormat));
  connect(settingsFrame, &SettingsFrame::requestUpdateRemoveIntermediateFiles,
          settings, qOverload<bool>(&Settings::removeIntermediateFiles));
  connect(settingsFrame, &SettingsFrame::requestUpdateContainerMode,
          settings, qOverload<bool>(&Settings::containerMode));

  // Receive settings changed
  connect(settings, qOverload<const QString&>(&Settings::cryptoProviderChanged),
          settingsFrame, &SettingsFrame::cryptoProviderChanged);
  connect(settings, qOverload<std::size_t>(&Settings::keySizeChanged),
          settingsFrame, &SettingsFrame::keySizeChanged);
  connect(settings, &Settings::compressionFormatChanged,
          settingsFrame, &SettingsFrame::compressionFormatChanged);
  connect(settings, &Settings::removeIntermediateFilesChanged,
          settingsFrame, &SettingsFrame::removeIntermediateFilesChanged);
  connect(settings, &Settings::containerModeChanged,
          settingsFrame, &SettingsFrame::containerModeChanged);

  // Output frame
  connect(outputFrame, &OutputFrame::requestUpdateOutputPath,
          settings, qOverload<const QString&>(&Settings::outputPath));
  connect(settings,
          &Settings::outputPathChanged,
          outputFrame,
          qOverload<const QString&>(&OutputFrame::outputPath));
  connect(outputFrame, &OutputFrame::selectOutputDir,
          this, &MainWindow::selectOutputDir);

  // Forwarded connections
  connect(fileListFrame, &FileListFrame::stopFile,
          this, &MainWindow::stopFile, Qt::DirectConnection);
  connect(controlButtonFrame, &ControlButtonFrame::processFiles,
          this, &MainWindow::processFiles);

  connect(this, &MainWindow::requestUpdateInputPath,
          settings, qOverload<const QString&>(&Settings::inputPath));
  connect(this, &MainWindow::requestUpdateOutputPath,
          settings, qOverload<const QString&>(&Settings::outputPath));
  connect(this, &MainWindow::requestUpdateClosePosition,
          settings, qOverload<const QPoint&>(&Settings::position));
  connect(this, &MainWindow::requestUpdateCloseSize,
          settings, qOverload<const QSize&>(&Settings::size));
  connect(this, &MainWindow::requestUpdateCloseMaximized,
          settings, qOverload<bool>(&Settings::maximized));
}

MainWindow::~MainWindow() = default;

void MainWindow::addFiles() {
  Q_ASSERT(settings);
  Q_ASSERT(fileListFrame);

  if (!settings) {
    return;
  }

  // Open a file dialog to get files
  const QStringList fileNames =
    QFileDialog::getOpenFileNames(this, tr("Add Files"),
                                  settings->inputPath(),
                                  tr("Any files (*)"));

  if (!fileNames.isEmpty()) { // If files were selected, add them to the model
    for (const QString& fileName : fileNames) {
      const QFileInfo fileInfo(fileName);
      fileListFrame->addFileToModel(fileInfo);
    }

    // Save this directory for returning to later
    const QFileInfo lastFileInfo(fileNames.last());
    emit requestUpdateInputPath(lastFileInfo.absolutePath());
  }
}

void MainWindow::removeFiles() {
  Q_ASSERT(fileListFrame);

  // Signal to cancel current cipher operation if it's in progress
  emit cancel();

  fileListFrame->clear();
}

void MainWindow::processFiles(const CryptDirection direction) {
  Q_D(MainWindow);
  Q_ASSERT(settings);
  Q_ASSERT(passwordFrame);
  Q_ASSERT(fileListFrame);

  const QString passphrase = passwordFrame->password();

  if (passphrase.isEmpty()) {
    // Inform user that a password is required to encrypt or decrypt
    updateStatusMessage(d->messages[0]);
    return;
  }

  const int rowCount = fileListFrame->rowCount();

  if (rowCount < 1) {
    return;
  }

  std::vector<QFileInfo> files;

  for (int row = 0; row < rowCount; ++row) {
    auto item = fileListFrame->item(row);
    const QFileInfo fileInfo(item->data().toString());
    files.push_back(fileInfo);
  }

  const QString outputPath = settings->outputPath();
  const QDir outputDir(outputPath);

  if (CryptDirection::Encrypt == direction) {
    EncryptConfig config;
    config.provider = settings->cryptoProvider();
    config.compressionFormat = settings->compressionFormat();
    config.passphrase = passphrase;
    config.cipher = settings->cipher();
    config.keySize = settings->keySize();
    config.modeOfOperation = settings->modeOfOperation();
    config.removeIntermediateFiles = settings->removeIntermediateFiles();

    emit encrypt(config, files, outputDir);
  } else {
    DecryptConfig config;
    config.passphrase = passphrase;
    config.removeIntermediateFiles = settings->removeIntermediateFiles();

    emit decrypt(config, files, outputDir);
  }
}

void MainWindow::updateFileProgress(const QFileInfo& fileInfo,
                                    const QString& task,
                                    const qint64 progressValue) {
  Q_ASSERT(fileListFrame);

  fileListFrame->updateProgress(fileInfo, task, progressValue);
}

void MainWindow::updateStatusMessage(const QString& message) {
  Q_ASSERT(messageFrame);

  messageFrame->appendMessage(message);
}

void MainWindow::updateError(const QString& message,
                             const QFileInfo& fileInfo) {
  if (message.contains(QStringLiteral("%1"))) {
    updateStatusMessage(message.arg(fileInfo.absoluteFilePath()));
  } else {
    updateStatusMessage(message);
  }

  updateFileProgress(QFileInfo(fileInfo.absoluteFilePath()), QString(), 0);
}

void MainWindow::selectOutputDir() {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  // Open a directory select dialog to get the output directory
  const QString outputDir =
    QFileDialog::getExistingDirectory(this, tr("Select Folder"),
                                      settings->outputPath());

  if (!outputDir.isEmpty()) { // Save this directory for returning to later
    emit requestUpdateOutputPath(outputDir);
  }
}

void MainWindow::settingsImported() {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  settingsFrame->init(settings->cryptoProvider(),
                      settings->compressionFormat(),
                      settings->keySize(),
                      settings->removeIntermediateFiles(),
                      settings->containerMode());

  outputFrame->outputPath(settings->outputPath());
}

QString MainWindow::loadStyleSheet(const QString& styleFilePath,
                                   const QString& defaultStyleFilePath) const {
  // Try to load user style, if it exists
  QFileInfo userStyleFileInfo(styleFilePath);
  QFileInfo defaultStyleFileInfo(defaultStyleFilePath);

  QByteArray styleSheetByteArray;

  const int ec = readConfigFile(userStyleFileInfo, styleSheetByteArray);

  if (ec < 1) {
    readConfigFile(defaultStyleFileInfo, styleSheetByteArray);
  }

  return QString(styleSheetByteArray);
}

} // namespace Kryvo
