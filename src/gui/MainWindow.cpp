/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "gui/MainWindow.hpp"
#include "gui/HeaderFrame.hpp"
#include "gui/FileListFrame.hpp"
#include "gui/MessageFrame.hpp"
#include "gui/PasswordFrame.hpp"
#include "gui/ControlButtonFrame.hpp"
#include "settings/Settings.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtGui/QDropEvent>
#include <QtGui/QIcon>
#include <QtCore/QModelIndexList>
#include <QtCore/QModelIndex>
#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QString>

class MainWindow::MainWindowPrivate {
 public:
  /*!
   * \brief MainWindowPrivate Constructs the MainWindow private implementation.
   * Initializes widgets, layouts, and settings.
   */
  explicit MainWindowPrivate();

  /*!
   * \brief loadStyleSheet Attempts to load a Qt stylesheet from the local
   * themes folder with the name specified in the local settings file. If the
   * load fails, the method will load the default stylesheet from the
   * application resources.
   * \param styleFile String representing the name of the stylesheet without
   * a file extension.
   * \return String containing the stylesheet file contents.
   */
  QString loadStyleSheet(const QString& styleFile);

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

  HeaderFrame* headerFrame;
  FileListFrame* fileListFrame;
  MessageFrame* messageFrame;
  PasswordFrame* passwordFrame;
  ControlButtonFrame* controlButtonFrame;

  // Settings
  Settings settings;

  // Messages to display to user
  const QStringList messages;

 private:
  // The busy status, when set to true, indicates that this object is currently
  // executing a cipher operation. The status allows the GUI to decide whether
  // to send new encryption/decryption requests.
  bool busyStatus;
};

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow{parent}, pimpl{make_unique<MainWindowPrivate>()}
{
  auto mainFrame = new QFrame{this};
  mainFrame->setObjectName("mainFrame");
  auto mainLayout = new QVBoxLayout{mainFrame};

  auto contentFrame = new QFrame{mainFrame};
  contentFrame->setObjectName("contentFrame");
  auto contentLayout = new QVBoxLayout{contentFrame};

  pimpl->headerFrame = new HeaderFrame{contentFrame};
  pimpl->headerFrame->setObjectName("headerFrame");
  contentLayout->addWidget(pimpl->headerFrame);

  pimpl->fileListFrame = new FileListFrame{contentFrame};
  pimpl->fileListFrame->setObjectName("fileListFrame");
  pimpl->fileListFrame->setSizePolicy(QSizePolicy::Expanding,
                                      QSizePolicy::Expanding);
  contentLayout->addWidget(pimpl->fileListFrame, 20);

  // Message text edit display
  pimpl->messageFrame = new MessageFrame{contentFrame};
  pimpl->messageFrame->setObjectName("message");
  pimpl->messageFrame->setSizePolicy(QSizePolicy::Expanding,
                                     QSizePolicy::Preferred);
  pimpl->messageFrame->setContentsMargins(0, 0, 0, 0);
  contentLayout->addWidget(pimpl->messageFrame, 1);

  // Password entry frame
  pimpl->passwordFrame = new PasswordFrame{contentFrame};
  contentLayout->addWidget(pimpl->passwordFrame);

  // Encrypt and decrypt control button frame
  pimpl->controlButtonFrame = new ControlButtonFrame{contentFrame};
  contentLayout->addWidget(pimpl->controlButtonFrame);

  mainLayout->addWidget(contentFrame);

  this->setCentralWidget(mainFrame);

  // Actions

  // Add files action
  auto addFilesAction = new QAction(this);
  addFilesAction->setShortcut(Qt::Key_O | Qt::CTRL);

  connect(addFilesAction, &QAction::triggered,
          this, &MainWindow::addFiles);
  this->addAction(addFilesAction);

  // Quit action
  auto quitAction = new QAction(this);
  quitAction->setShortcut(Qt::Key_Q | Qt::CTRL);

  connect(quitAction, &QAction::triggered,
          this, &QMainWindow::close);
  this->addAction(quitAction);

  // Header tool connections
  connect(pimpl->headerFrame, &HeaderFrame::pause,
          this, &MainWindow::pauseCipher);
  connect(pimpl->headerFrame, &HeaderFrame::addFiles,
          this, &MainWindow::addFiles);
  connect(pimpl->headerFrame, &HeaderFrame::removeFiles,
          this, &MainWindow::removeFiles);

  // Forwarded connections
  connect(pimpl->fileListFrame, &FileListFrame::stopFile,
          this, &MainWindow::stopFile, Qt::DirectConnection);
  connect(pimpl->controlButtonFrame, &ControlButtonFrame::encryptFiles,
          this, &MainWindow::encryptFiles);
  connect(pimpl->controlButtonFrame, &ControlButtonFrame::decryptFiles,
          this, &MainWindow::decryptFiles);

  // Set object name
  this->setObjectName("MainWindow");

  // Title
  this->setWindowTitle(tr("Kryvos"));

  // Load stylesheet
  const auto styleSheet = pimpl->loadStyleSheet(pimpl->settings.
                                                  styleSheetPath());

  if (!styleSheet.isEmpty())
  {
    this->setStyleSheet(styleSheet);
  }

  this->move(pimpl->settings.position());

  if (pimpl->settings.maximized())
  { // Move window, then maximize to ensure maximize occurs on correct screen
    this->setWindowState(this->windowState() | Qt::WindowMaximized);
  }
  else
  {
    this->resize(pimpl->settings.size());
  }

  // Enable drag and drop
  this->setAcceptDrops(true);
}

MainWindow::~MainWindow() {}

void MainWindow::addFiles()
{
  Q_ASSERT(pimpl);

  // Open a file dialog to get files
  const auto files = QFileDialog::getOpenFileNames(this,
                                                   tr("Add Files"),
                                                   pimpl->settings.
                                                     lastDirectory(),
                                                   tr("Any files (*)"));

  if (!files.isEmpty())
  { // If files were selected, add them to the model
    for (const auto& file : files)
    {
      pimpl->fileListFrame->addFileToModel(file);
    }

    // Save this directory for returning to later
    const auto fileName = files[0];
    QFileInfo file{fileName};
    pimpl->settings.lastDirectory(file.absolutePath());
  }
}

void MainWindow::removeFiles()
{
  Q_ASSERT(pimpl);

  // Signal to abort current cipher operation if it's in progress
  emit abortCipher();

  pimpl->fileListFrame->clear();
}

void MainWindow::encryptFiles()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->passwordFrame);
  Q_ASSERT(pimpl->fileListModel);

  if (!pimpl->isBusy())
  {
    // Get passphrase from line edit
    const auto passphrase = pimpl->passwordFrame->password();

    if (!passphrase.isEmpty())
    {
      const auto rowCount = pimpl->fileListFrame->rowCount();
      if (rowCount > 0)
      {
        auto fileList = QStringList{};

        for (auto row = 0; row < rowCount; ++row)
        {
          auto item = pimpl->fileListFrame->item(row);
          fileList.append(item->text());
        }

        // Start encrypting the file list
        emit encrypt(passphrase, fileList, pimpl->settings.lastAlgorithm());
      }
    }
    else
    { // Inform user that a password is required to encrypt or decrypt
      this->updateStatusMessage(pimpl->messages[0]);
    }
  }
  else
  {
    this->updateStatusMessage(pimpl->messages[1]);
  }
}

void MainWindow::decryptFiles()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->passwordFrame);
  Q_ASSERT(pimpl->fileListModel);

  if (!pimpl->isBusy())
  {
    // Get passphrase from line edit
    const auto passphrase = pimpl->passwordFrame->password();

    if (!passphrase.isEmpty())
    {
      const auto rowCount = pimpl->fileListFrame->rowCount();
      if (rowCount > 0)
      {
        auto fileList = QStringList{};

        for (auto row = 0; row < rowCount; ++row)
        {
          auto item = pimpl->fileListFrame->item(row);
          fileList.append(item->text());
        }

        // Start decrypting the file list
        emit decrypt(passphrase, fileList);
      }
    }
    else
    { // Inform user that a password is required to encrypt or decrypt
      this->updateStatusMessage(pimpl->messages[0]);
    }
  }
  else
  {
    this->updateStatusMessage(pimpl->messages[1]);
  }
}

void MainWindow::updateProgress(const QString& path, qint64 percent)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListFrame);

  pimpl->fileListFrame->updateProgress(path, percent);
}

void MainWindow::updateStatusMessage(const QString& message)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->messageFrame);

  pimpl->messageFrame->appendPlainText(message);
}

void MainWindow::updateError(const QString& path, const QString& message)
{
  this->updateStatusMessage(message);
  this->updateProgress(path, 0);
}

void MainWindow::updateBusyStatus(bool busy)
{
  Q_ASSERT(pimpl);

  pimpl->busy(busy);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  pimpl->settings.position(this->pos());

  if (this->isMaximized())
  {
    pimpl->settings.maximized(true);
  }
  else
  {
    pimpl->settings.maximized(false);
    pimpl->settings.size(this->size());
  }

  QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  // Show drag and drop as a move action
  event->setDropAction(Qt::MoveAction);

  if (event->mimeData()->hasUrls())
  { // Accept drag and drops with files only
    event->accept();
  }
}

void MainWindow::dropEvent(QDropEvent* event)
{
  Q_ASSERT(pimpl);

  // Check for the URL MIME type, which is a list of files
  if (event->mimeData()->hasUrls())
  { // Extract the local path from the file(s)
    for (const auto& url : event->mimeData()->urls())
    {
      pimpl->fileListFrame->addFileToModel(url.toLocalFile());
    }
  }
}

QSize MainWindow::sizeHint() const
{
  return QSize(800, 600);
}

QSize MainWindow::minimumSizeHint() const
{
  return QSize(600, 350);
}

MainWindow::MainWindowPrivate::MainWindowPrivate() :
  headerFrame{nullptr},
  fileListFrame{nullptr},
  messageFrame{nullptr},
  passwordFrame{nullptr},
  messages{tr("A password is required to encrypt or decrypt files. Please enter"
              " one to continue."),
           tr("Encryption/decryption is already in progress. Please wait until"
              " the current operation finishes.")},
  busyStatus{false} {}

QString MainWindow::MainWindowPrivate::loadStyleSheet(const QString& styleFile)
{
  // Try to load user theme, if it exists
  const auto styleSheetPath = QString{"themes/" + styleFile};
  QFile userTheme{styleSheetPath};

  auto styleSheet = QString{};

  if (userTheme.exists())
  {
    auto themeOpen = userTheme.open(QFile::ReadOnly);

    if (themeOpen)
    {
      styleSheet = QLatin1String(userTheme.readAll());
      userTheme.close();
    }
  }
  else
  { // Otherwise, load default theme
    QFile defaultTheme{":/stylesheets/kryvos.qss"};

    auto defaultThemeOpen = defaultTheme.open(QFile::ReadOnly);

    if (defaultThemeOpen)
    {
      styleSheet = QLatin1String(defaultTheme.readAll());
      defaultTheme.close();
    }
  }

  return styleSheet;
}

void MainWindow::MainWindowPrivate::busy(bool busy)
{
  busyStatus = busy;
}

bool MainWindow::MainWindowPrivate::isBusy() const
{
  return busyStatus;
}
