#include "MainWindow.hpp"
#include "SlidingStackedWidget.hpp"
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

  connect(outputFrame, &OutputFrame::outputPathChanged,
          this, &MainWindow::outputPathChanged);
  connect(outputFrame, &OutputFrame::selectOutputDir,
          this, &MainWindow::selectOutputDir);

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

  // Settings frame connections
  connect(settingsFrame, &SettingsFrame::updateKeySize,
          this, &MainWindow::updateKeySize);

  connect(settingsFrame, &SettingsFrame::updateCompressionFormat,
          this, &MainWindow::updateCompressionFormat);

  connect(settingsFrame, &SettingsFrame::updateRemoveIntermediateFiles,
          this, &MainWindow::updateRemoveIntermediateFiles);

  connect(settingsFrame, &SettingsFrame::updateContainerMode,
          this, &MainWindow::updateContainerMode);

  // Forwarded connections
  connect(fileListFrame, &FileListFrame::stopFile,
          this, &MainWindow::stopFile, Qt::DirectConnection);
  connect(controlButtonFrame, &ControlButtonFrame::processFiles,
          this, &MainWindow::processFiles);
}

MainWindow::~MainWindow() {
  settings->outputPath(outputFrame->outputPath());
}

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
    settings->inputPath(lastFileInfo.absolutePath());
  }
}

void MainWindow::removeFiles() {
  Q_ASSERT(fileListFrame);

  // Signal to abort current cipher operation if it's in progress
  emit abort();

  fileListFrame->clear();
}

void MainWindow::processFiles(
  const CryptDirection direction) {
  Q_D(MainWindow);
  Q_ASSERT(settings);
  Q_ASSERT(passwordFrame);
  Q_ASSERT(fileListFrame);

  const QString passphrase = passwordFrame->password();

  if (!passphrase.isEmpty()) {
    const int rowCount = fileListFrame->rowCount();

    if (rowCount > 0) {
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
  } else { // Inform user that a password is required to encrypt or decrypt
    updateStatusMessage(d->messages[0]);
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

void MainWindow::updateCipher(const QString& cipher) {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  settings->cipher(cipher);
}

void MainWindow::updateKeySize(const std::size_t keySize) {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  settings->keySize(keySize);
}

void MainWindow::updateCompressionFormat(const QString& format) {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  settings->compressionFormat(format);
}

void MainWindow::updateRemoveIntermediateFiles(const bool removeIntermediate) {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  settings->removeIntermediateFiles(removeIntermediate);
}

void MainWindow::updateContainerMode(const bool container) {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  settings->containerMode(container);
}

void MainWindow::outputPathChanged(const QString& path) {
  Q_ASSERT(settings);

  if (!settings) {
    return;
  }

  if (!path.isEmpty()) {
    settings->outputPath(path);
  }
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
    settings->outputPath(outputDir);
    outputFrame->outputPath(outputDir);
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

QString MainWindow::loadStyleSheet(const QString& styleFile,
                                   const QString& defaultFile) const {
  // Try to load user theme, if it exists
  const QString styleSheetPath = QStringLiteral("themes") %
                                 QStringLiteral("/") % styleFile;
  QFile userTheme(styleSheetPath);

  QString styleSheet;

  if (userTheme.exists()) {
    const bool themeOpen = userTheme.open(QFile::ReadOnly);

    if (themeOpen) {
      styleSheet = QString(userTheme.readAll());
      userTheme.close();
    }
  } else { // Otherwise, load default theme
    const QString localPath = QStringLiteral(":/stylesheets/") % defaultFile;
    QFile defaultTheme(localPath);

    const bool defaultThemeOpen = defaultTheme.open(QFile::ReadOnly);

    if (defaultThemeOpen) {
      styleSheet = QString(defaultTheme.readAll());
      defaultTheme.close();
    }
  }

  return styleSheet;
}

} // namespace Kryvo
