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

#include "DesktopMainWindow.hpp"
#include <QtWidgets/QAction>
#include <QtGui/QDropEvent>
#include <QtCore/QMimeData>

DesktopMainWindow::DesktopMainWindow(Settings* settings, QWidget* parent) :
  MainWindow{settings, parent}
{
  this->settings = settings;

  messageFrame->setText(tr("To begin, click the Add Files button or drag and "
                           "drop files. Enter a password. Finally, click "
                           "the Encrypt or Decrypt button."));

  // Adjust stretch of file list view
  contentLayout->setStretch(1, 20);
  // Adjust stretch of message box
  contentLayout->setStretch(2, 1);

  // Add files action
  auto addFilesAction = new QAction{this};
  addFilesAction->setShortcut(Qt::Key_O | Qt::CTRL);
  connect(addFilesAction, &QAction::triggered,
          this, &MainWindow::addFiles);
  this->addAction(addFilesAction);

  // Quit action
  auto quitAction = new QAction{this};
  quitAction->setShortcut(Qt::Key_Q | Qt::CTRL);
  connect(quitAction, &QAction::triggered,
          this, &QMainWindow::close);
  this->addAction(quitAction);

  this->move(this->settings->position());

  if (this->settings->maximized())
  { // Move window, then maximize to ensure maximize occurs on correct screen
    this->setWindowState(this->windowState() | Qt::WindowMaximized);
  }
  else
  {
    this->resize(this->settings->size());
  }

  // Enable drag and drop
  this->setAcceptDrops(true);

  // Load stylesheet
  const auto styleSheet = loadStyleSheet(this->settings->styleSheetPath(),
                                         "kryvos.qss");

  if (!styleSheet.isEmpty())
  {
    this->setStyleSheet(styleSheet);
  }
}

DesktopMainWindow::~DesktopMainWindow() {}

void DesktopMainWindow::closeEvent(QCloseEvent* event)
{
  this->settings->position(this->pos());

  if (this->isMaximized())
  {
    this->settings->maximized(true);
  }
  else
  {
    this->settings->maximized(false);
    this->settings->size(this->size());
  }

  QMainWindow::closeEvent(event);
}

void DesktopMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  // Show drag and drop as a move action
  event->setDropAction(Qt::MoveAction);

  if (event->mimeData()->hasUrls())
  { // Accept drag and drops with files only
    event->accept();
  }
}

void DesktopMainWindow::dropEvent(QDropEvent* event)
{
  // Check for the URL MIME type, which is a list of files
  if (event->mimeData()->hasUrls())
  { // Extract the local path from the file(s)
    for (const auto& url : event->mimeData()->urls())
    {
      fileListFrame->addFileToModel(url.toLocalFile());
    }
  }
}

QSize DesktopMainWindow::sizeHint() const
{
  return QSize(800, 600);
}

QSize DesktopMainWindow::minimumSizeHint() const
{
  return QSize(600, 350);
}
