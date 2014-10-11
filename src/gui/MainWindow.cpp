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
#include "utility/make_unique.h"
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QStackedLayout>
#include <QtGui/QIcon>
#include <QtCore/QStateMachine>
#include <QtCore/QState>
#include <QtCore/QModelIndexList>
#include <QtCore/QModelIndex>
#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QStringRef>
#include <QtCore/QStringBuilder>
#include <QtCore/QString>

class MainWindow::MainWindowPrivate {
 public:
  /*!
   * \brief MainWindowPrivate Constructs the MainWindow private implementation.
   * Initializes widgets, layouts, and settings.
   */
  explicit MainWindowPrivate();

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

  Settings* settings;

  // Messages to display to user
  const QStringList messages;

  // GUI state machine
  std::unique_ptr<QStateMachine> stateMachine;

 private:
  // The busy status, when set to true, indicates that this object is currently
  // executing a cipher operation. The status allows the GUI to decide whether
  // to send new encryption/decryption requests.
  bool busyStatus;
};

MainWindow::MainWindow(Settings* settings, QWidget* parent) :
  QMainWindow{parent}, headerFrame{nullptr}, fileListFrame{nullptr},
  messageFrame{nullptr}, passwordFrame{nullptr}, controlButtonFrame{nullptr},
  contentLayout{nullptr}, pimpl{make_unique<MainWindowPrivate>()}
{
  // Set object name
  this->setObjectName("MainWindow");

  // Title
  this->setWindowTitle(tr("Kryvos"));

  // Store settings object
  pimpl->settings = settings;

  auto mainFrame = new QFrame{this};
  mainFrame->setObjectName("mainFrame");
  auto mainLayout = new QVBoxLayout{mainFrame};

  settingsFrame = new SettingsFrame{mainFrame};
  settingsFrame->setObjectName("settingsFrame");
  mainLayout->addWidget(settingsFrame);

  auto contentFrame = new QFrame{mainFrame};
  contentFrame->setObjectName("contentFrame");

  headerFrame = new HeaderFrame{contentFrame};
  headerFrame->setObjectName("headerFrame");

  fileListFrame = new FileListFrame{contentFrame};
  fileListFrame->setObjectName("fileListFrame");
  fileListFrame->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);

  // Message text edit display
  messageFrame = new MessageFrame{contentFrame};
  messageFrame->setObjectName("message");
  messageFrame->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Preferred);
  messageFrame->setContentsMargins(0, 0, 0, 0);

  // Password entry frame
  passwordFrame = new PasswordFrame{contentFrame};
  passwordFrame->setObjectName("passwordFrame");

  // Encrypt and decrypt control button frame
  controlButtonFrame = new ControlButtonFrame{contentFrame};
  controlButtonFrame->setObjectName("controlButtonFrame");

  contentLayout = new QVBoxLayout{contentFrame};
  contentLayout->addWidget(headerFrame);
  contentLayout->addWidget(fileListFrame);
  contentLayout->addWidget(messageFrame);
  contentLayout->addWidget(passwordFrame);
  contentLayout->addWidget(controlButtonFrame);

  mainLayout->addWidget(contentFrame);

  this->setCentralWidget(mainFrame);

  // Header button connections
  connect(headerFrame, &HeaderFrame::pause,
          this, &MainWindow::pauseCipher);
  connect(headerFrame, &HeaderFrame::addFiles,
          this, &MainWindow::addFiles);
  connect(headerFrame, &HeaderFrame::removeFiles,
          this, &MainWindow::removeFiles);

  // Forwarded connections
  connect(fileListFrame, &FileListFrame::stopFile,
          this, &MainWindow::stopFile, Qt::DirectConnection);
  connect(controlButtonFrame, &ControlButtonFrame::processFiles,
          this, &MainWindow::processFiles);

  // GUI states
  QState* mainState = new QState{};
  mainState->assignProperty(contentFrame, "visible", true);
  mainState->assignProperty(settingsFrame, "visible", false);

  QState* settingsState = new QState{};
  settingsState->assignProperty(contentFrame, "visible", false);
  settingsState->assignProperty(settingsFrame, "visible", true);

  mainState->addTransition(headerFrame,
                           SIGNAL(switchFrame()),
                           settingsState);
  settingsState->addTransition(settingsFrame,
                               SIGNAL(switchFrame()),
                               mainState);

  pimpl->stateMachine->addState(mainState);
  pimpl->stateMachine->addState(settingsState);
  pimpl->stateMachine->setInitialState(mainState);

  pimpl->stateMachine->start();
}

MainWindow::~MainWindow() {}

void MainWindow::addFiles()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->settings);
  Q_ASSERT(fileListFrame);

  // Open a file dialog to get files
  const auto files = QFileDialog::getOpenFileNames(this,
                                                   tr("Add Files"),
                                                   pimpl->settings->
                                                     lastDirectory(),
                                                   tr("Any files (*)"));

  if (!files.isEmpty())
  { // If files were selected, add them to the model
    for (const auto& file : files)
    {
      fileListFrame->addFileToModel(file);
    }

    // Save this directory for returning to later
    const auto fileName = files[0];
    QFileInfo file{fileName};
    pimpl->settings->lastDirectory(file.absolutePath());
  }
}

void MainWindow::removeFiles()
{
  Q_ASSERT(fileListFrame);

  // Signal to abort current cipher operation if it's in progress
  emit abortCipher();

  fileListFrame->clear();
}

void MainWindow::processFiles(bool cryptFlag)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->settings);
  Q_ASSERT(passwordFrame);
  Q_ASSERT(fileListFrame);

  if (!pimpl->isBusy())
  {
    // Get passphrase from line edit
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

        if (cryptFlag)
        {
          // Start encrypting the file list
          emit encrypt(passphrase, fileList, pimpl->settings->lastAlgorithm());
        }
        else
        {
          // Start decrypting the file list
          emit decrypt(passphrase, fileList);
        }
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
  Q_ASSERT(fileListFrame);

  fileListFrame->updateProgress(path, percent);
}

void MainWindow::updateStatusMessage(const QString& message)
{
  Q_ASSERT(messageFrame);

  messageFrame->appendPlainText(message);
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

QString MainWindow::loadStyleSheet(const QString& styleFile,
                                   const QString& defaultFile)
{
  // Try to load user theme, if it exists
  const QString styleSheetPath = QLatin1String{"themes"} % QDir::separator() %
          styleFile;
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
    QFile defaultTheme{QLatin1String{":/stylesheets/"} % defaultFile};

    auto defaultThemeOpen = defaultTheme.open(QFile::ReadOnly);

    if (defaultThemeOpen)
    {
      styleSheet = QLatin1String(defaultTheme.readAll());
      defaultTheme.close();
    }
  }

  return styleSheet;
}

MainWindow::MainWindowPrivate::MainWindowPrivate() :
  settings{nullptr},
  messages{tr("A password is required to encrypt or decrypt files. Please enter"
              " one to continue."),
           tr("Encryption/decryption is already in progress. Please wait until"
              " the current operation finishes.")},
  stateMachine{make_unique<QStateMachine>()},
  busyStatus{false} {}

void MainWindow::MainWindowPrivate::busy(bool busy)
{
  busyStatus = busy;
}

bool MainWindow::MainWindowPrivate::isBusy() const
{
  return busyStatus;
}
