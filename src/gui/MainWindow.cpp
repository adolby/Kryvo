#include "src/gui/MainWindow.hpp"
#include "src/gui/SlidingStackedWidget.hpp"
#include "src/utility/pimpl_impl.h"
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QBoxLayout>
#include <QModelIndexList>
#include <QModelIndex>
#include <QMimeData>
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

class MainWindow::MainWindowPrivate {
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

  Settings* settings;

  // Messages to display to user
  const QStringList messages;

 private:
  // The busy status, when set to true, indicates that the cryptography object
  // is currently executing a cipher operation. The status allows the GUI to
  // decide whether to send new encryption/decryption requests.
  bool busyStatus;
};

MainWindow::MainWindow(Settings* settings, QWidget* parent)
  : QMainWindow{parent}, headerFrame{nullptr}, fileListFrame{nullptr},
    messageFrame{nullptr}, outputFrame{nullptr}, passwordFrame{nullptr},
    controlButtonFrame{nullptr}, contentLayout{nullptr}
{
  // Set object name
  this->setObjectName(QStringLiteral("mainWindow"));

  // Title
  this->setWindowTitle(tr("Kryvos"));

  // Keep settings object
  m->settings = settings;

  auto slidingStackedWidget = new SlidingStackedWidget{this};

  auto contentFrame = new QFrame{slidingStackedWidget};
  contentFrame->setObjectName(QStringLiteral("contentFrame"));

  headerFrame = new HeaderFrame{contentFrame};
  headerFrame->setObjectName(QStringLiteral("headerFrame"));
  headerFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  fileListFrame = new FileListFrame{contentFrame};
  fileListFrame->setObjectName(QStringLiteral("fileListFrame"));
  fileListFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Message text edit display
  messageFrame = new MessageFrame{contentFrame};
  messageFrame->setObjectName(QStringLiteral("messageFrame"));
  messageFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  messageFrame->setMinimumHeight(36);

  // Archive file name frame
  outputFrame = new OutputFrame{contentFrame};
  outputFrame->setObjectName(QStringLiteral("outputFrame"));
  outputFrame->outputPath(m->settings->outputPath());

  // Password entry frame
  passwordFrame = new PasswordFrame{contentFrame};
  passwordFrame->setObjectName(QStringLiteral("passwordFrame"));

  // Encrypt and decrypt control button frame
  controlButtonFrame = new ControlButtonFrame{contentFrame};
  controlButtonFrame->setObjectName(QStringLiteral("controlButtonFrame"));

  contentLayout = new QVBoxLayout{contentFrame};
  contentLayout->addWidget(headerFrame);
  contentLayout->addWidget(fileListFrame);
  contentLayout->addWidget(messageFrame);
  contentLayout->addWidget(outputFrame);
  contentLayout->addWidget(passwordFrame);
  contentLayout->addWidget(controlButtonFrame);
  contentLayout->setSpacing(0);

  slidingStackedWidget->addWidget(contentFrame);

  settingsFrame = new SettingsFrame{m->settings->cipher(),
                                    m->settings->keySize(),
                                    m->settings->modeOfOperation(),
                                    m->settings->compressionMode(),
                                    m->settings->containerMode(),
                                    slidingStackedWidget};
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

MainWindow::~MainWindow()
{
  m->settings->outputPath(outputFrame->outputPath());
}

void MainWindow::addFiles()
{
  Q_ASSERT(m->settings);
  Q_ASSERT(fileListFrame);

  // Open a file dialog to get files
  const auto fileNames =
    QFileDialog::getOpenFileNames(this, tr("Add Files"),
                                  m->settings->lastOpenPath(),
                                  tr("Any files (*)"));

  if (!fileNames.isEmpty())
  { // If files were selected, add them to the model
    for (const auto& fileName : fileNames)
    {
      const QFileInfo fileInfo{fileName};
      fileListFrame->addFileToModel(fileInfo.absoluteFilePath());
    }

    // Save this directory for returning to later
    const QFileInfo lastFileInfo{fileNames.last()};
    m->settings->lastOpenPath(lastFileInfo.absolutePath());
  }
}

void MainWindow::removeFiles()
{
  Q_ASSERT(fileListFrame);

  // Signal to abort current cipher operation if it's in progress
  emit abortCipher();

  fileListFrame->clear();
}

void MainWindow::processFiles(const bool cryptDirection)
{
  Q_ASSERT(m->settings);
  Q_ASSERT(passwordFrame);
  Q_ASSERT(fileListFrame);

  if (!m->isBusy())
  {
    const auto outputPath = outputFrame->outputPath();
    const auto passphrase = passwordFrame->password();

    if (!passphrase.isEmpty())
    {
      const auto rowCount = fileListFrame->rowCount();

      if (rowCount > 0)
      {
        auto fileList = QStringList{};

        for (auto row = 0; row < rowCount; ++row)
        {
          auto item = fileListFrame->item(row);
          fileList.append(item->data().toString());
        }

        if (cryptDirection)
        {
          emit encrypt(passphrase,
                       fileList,
                       outputPath,
                       m->settings->cipher(),
                       m->settings->keySize(),
                       m->settings->modeOfOperation(),
                       m->settings->compressionMode(),
                       m->settings->containerMode());
        }
        else
        {
          emit decrypt(passphrase, fileList, outputPath);
        }
      }
    }
    else
    { // Inform user that a password is required to encrypt or decrypt
      updateStatusMessage(m->messages[0]);
    }
  }
  else
  {
    // Inform user that encryption/decryption is already in progress
    updateStatusMessage(m->messages[1]);
  }
}

void MainWindow::updateProgress(const QString& path, const qint64 percent)
{
  Q_ASSERT(fileListFrame);

  fileListFrame->updateProgress(path, percent);
}

void MainWindow::updateStatusMessage(const QString& message)
{
  Q_ASSERT(messageFrame);

  messageFrame->appendText(message);
}

void MainWindow::updateError(const QString& message, const QString& fileName)
{
  if (!fileName.isEmpty())
  {
    updateStatusMessage(message.arg(fileName));
    updateProgress(fileName, 0);
  }
  else
  {
    updateStatusMessage(message);
  }
}

void MainWindow::updateBusyStatus(const bool busy)
{
  m->busy(busy);
}

void MainWindow::updateCipher(const QString& cipher)
{
  m->settings->cipher(cipher);
}

void MainWindow::updateKeySize(const std::size_t& keySize)
{
  m->settings->keySize(keySize);
}

void MainWindow::updateModeOfOperation(const QString& mode)
{
  m->settings->modeOfOperation(mode);
}

void MainWindow::updateCompressionMode(const bool compress)
{
  m->settings->compressionMode(compress);
}

void MainWindow::updateContainerMode(const bool container)
{
  m->settings->containerMode(container);
}

QString MainWindow::loadStyleSheet(const QString& styleFile,
                                   const QString& defaultFile) const
{
  // Try to load user theme, if it exists
  const QString styleSheetPath = QStringLiteral("themes") %
                                 QDir::separator() % styleFile;
  QFile userTheme{styleSheetPath};

  auto styleSheet = QString{};

  if (userTheme.exists())
  {
    auto themeOpen = userTheme.open(QFile::ReadOnly);

    if (themeOpen)
    {
      styleSheet = QString{QLatin1String{userTheme.readAll()}};
      userTheme.close();
    }
  }
  else
  { // Otherwise, load default theme
    const QString localPath = QStringLiteral(":/stylesheets/") % defaultFile;
    QFile defaultTheme{localPath};

    auto defaultThemeOpen = defaultTheme.open(QFile::ReadOnly);

    if (defaultThemeOpen)
    {
      styleSheet = QString{QLatin1String{defaultTheme.readAll()}};
      defaultTheme.close();
    }
  }

  return styleSheet;
}

MainWindow::MainWindowPrivate::MainWindowPrivate()
  : settings{nullptr},
    messages{std::initializer_list<QString>({tr("A password is required to "
                                                "encrypt or decrypt files. "
                                                "Please enter one to "
                                                "continue."),
                                             tr("Encryption/decryption is "
                                                "already in progress. "
                                                "Please wait until the current "
                                                "operation finishes.")})},
    busyStatus{false} {}

void MainWindow::MainWindowPrivate::busy(const bool busy)
{
  busyStatus = busy;
}

bool MainWindow::MainWindowPrivate::isBusy() const
{
  return busyStatus;
}
