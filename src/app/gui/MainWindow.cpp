#include "gui/MainWindow.hpp"
#include "gui/SlidingStackedWidget.hpp"
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
#include <QStringRef>
#include <QStringBuilder>
#include <QString>
#include <initializer_list>

class Kryvo::MainWindowPrivate {
  Q_DISABLE_COPY(MainWindowPrivate)

 public:
  /*!
   * \brief MainWindowPrivate Constructs the MainWindow private implementation.
   * Initializes widgets, layouts, and settings.
   */
  MainWindowPrivate();

  /*!
   * \brief busy Sets the busy status received from the cipher operation.
   * \param busy Boolean representing the busy status.
   */
  void busy(const bool busy);

  /*!
   * \brief isBusy Returns the busy status received from the cipher operation.
   * \return Boolean representing the busy status.
   */
  bool isBusy() const;

  // Messages to display to user
  const QStringList messages;

 private:
  // The busy status, when set to true, indicates that the cryptography object
  // is currently executing a cipher operation. The status allows the GUI to
  // decide whether to send new encryption/decryption requests.
  bool busyStatus;
};

Kryvo::MainWindow::MainWindow(Settings* s, QWidget* parent)
  : QMainWindow(parent), d_ptr(std::make_unique<MainWindowPrivate>()),
    headerFrame(nullptr), fileListFrame(nullptr), messageFrame(nullptr),
    outputFrame(nullptr), passwordFrame(nullptr), controlButtonFrame(nullptr),
    contentLayout(nullptr) {
  // Set object name
  this->setObjectName(QStringLiteral("mainWindow"));

  // Title
  this->setWindowTitle(tr("Kryvo"));

  // Keep settings object
  settings = s;

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
  outputFrame->outputPath(settings->outputPath());

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

  settingsFrame = new SettingsFrame(settings->cipher(),
                                    settings->keySize(),
                                    settings->modeOfOperation(),
                                    settings->compressionMode(),
                                    settings->containerMode(),
                                    slidingStackedWidget);
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
          this, &MainWindow::pauseCipher);
  connect(headerFrame, &HeaderFrame::addFiles,
          this, &MainWindow::addFiles);
  connect(headerFrame, &HeaderFrame::removeFiles,
          this, &MainWindow::removeFiles);

  // Settings frame connections
  connect(settingsFrame, &SettingsFrame::updateCipher,
          this, &MainWindow::updateCipher);

  connect(settingsFrame, &SettingsFrame::updateKeySize,
          this, &MainWindow::updateKeySize);

  connect(settingsFrame, &SettingsFrame::updateModeOfOperation,
          this, &MainWindow::updateModeOfOperation);

  connect(settingsFrame, &SettingsFrame::updateCompressionMode,
          this, &MainWindow::updateCompressionMode);

  connect(settingsFrame, &SettingsFrame::updateContainerMode,
          this, &MainWindow::updateContainerMode);

  // Forwarded connections
  connect(fileListFrame, &FileListFrame::stopFile,
          this, &MainWindow::stopFile, Qt::DirectConnection);
  connect(controlButtonFrame, &ControlButtonFrame::processFiles,
          this, &MainWindow::processFiles);
}

Kryvo::MainWindow::~MainWindow() {
  settings->outputPath(outputFrame->outputPath());
}

void Kryvo::MainWindow::addFiles() {
  Q_ASSERT(settings);
  Q_ASSERT(fileListFrame);

  // Open a file dialog to get files
  const QStringList fileNames =
    QFileDialog::getOpenFileNames(this, tr("Add Files"),
                                  settings->lastOpenPath(),
                                  tr("Any files (*)"));

  if (!fileNames.isEmpty()) { // If files were selected, add them to the model
    for (const QString& fileName : fileNames) {
      const QFileInfo fileInfo(fileName);
      fileListFrame->addFileToModel(fileInfo.absoluteFilePath());
    }

    // Save this directory for returning to later
    const QFileInfo lastFileInfo(fileNames.last());
    settings->lastOpenPath(lastFileInfo.absolutePath());
  }
}

void Kryvo::MainWindow::removeFiles() {
  Q_ASSERT(fileListFrame);

  // Signal to abort current cipher operation if it's in progress
  emit abortCipher();

  fileListFrame->clear();
}

void Kryvo::MainWindow::processFiles(const bool cryptDirection) {
  Q_D(MainWindow);
  Q_ASSERT(settings);
  Q_ASSERT(passwordFrame);
  Q_ASSERT(fileListFrame);

  if (!d->isBusy()) {
    const QString& outputPath = outputFrame->outputPath();
    const QString& passphrase = passwordFrame->password();

    if (!passphrase.isEmpty()) {
      const int rowCount = fileListFrame->rowCount();

      if (rowCount > 0) {
        QStringList fileList;

        for (int row = 0; row < rowCount; ++row) {
          auto item = fileListFrame->item(row);
          fileList.append(item->data().toString());
        }

        if (cryptDirection) {
          emit encrypt(passphrase,
                       fileList,
                       outputPath,
                       settings->cipher(),
                       settings->keySize(),
                       settings->modeOfOperation(),
                       settings->compressionMode(),
                       settings->containerMode());
        }
        else {
          emit decrypt(passphrase, fileList, outputPath);
        }
      }
    }
    else { // Inform user that a password is required to encrypt or decrypt
      updateStatusMessage(d->messages[0]);
    }
  }
  else {
    // Inform user that encryption/decryption is already in progress
    updateStatusMessage(d->messages[1]);
  }
}

void Kryvo::MainWindow::updateFileProgress(const QString& path,
                                           const QString& task,
                                           const qint64 progressValue) {
  Q_ASSERT(fileListFrame);

  fileListFrame->updateProgress(path, task, progressValue);
}

void Kryvo::MainWindow::updateStatusMessage(const QString& message) {
  Q_ASSERT(messageFrame);

  messageFrame->appendText(message);
}

void Kryvo::MainWindow::updateError(const QString& message,
                                    const QString& fileName) {
  if (message.contains(QStringLiteral("%1"))) {
    updateStatusMessage(message.arg(fileName));
  }
  else {
    updateStatusMessage(message);
  }

  updateFileProgress(fileName, QString(), 0);
}

void Kryvo::MainWindow::updateBusyStatus(const bool busy) {
  Q_D(MainWindow);
  d->busy(busy);
}

void Kryvo::MainWindow::updateCipher(const QString& cipher) {
  settings->cipher(cipher);
}

void Kryvo::MainWindow::updateKeySize(const std::size_t keySize) {
  settings->keySize(keySize);
}

void Kryvo::MainWindow::updateModeOfOperation(const QString& mode) {
  settings->modeOfOperation(mode);
}

void Kryvo::MainWindow::updateCompressionMode(const bool compress) {
  settings->compressionMode(compress);
}

void Kryvo::MainWindow::updateContainerMode(const bool container) {
  settings->containerMode(container);
}

QString Kryvo::MainWindow::loadStyleSheet(const QString& styleFile,
                                          const QString& defaultFile) const {
  // Try to load user theme, if it exists
  const QString styleSheetPath = QStringLiteral("themes") %
                                 QDir::separator() % styleFile;
  QFile userTheme(styleSheetPath);

  QString styleSheet;

  if (userTheme.exists()) {
    const bool themeOpen = userTheme.open(QFile::ReadOnly);

    if (themeOpen) {
      styleSheet = QString(userTheme.readAll());
      userTheme.close();
    }
  }
  else { // Otherwise, load default theme
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

Kryvo::MainWindowPrivate::MainWindowPrivate()
  : messages(std::initializer_list<QString>(
               {QObject::tr("A password is required to encrypt or decrypt "
                            "files. Please enter one to continue."),
                QObject::tr("Encryption/decryption is already in progress. "
                            "Please wait until the current operation "
                            "finishes.")})),
    busyStatus(false) {
}

void Kryvo::MainWindowPrivate::busy(const bool busy) {
  busyStatus = busy;
}

bool Kryvo::MainWindowPrivate::isBusy() const {
  return busyStatus;
}
