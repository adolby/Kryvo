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

#include "gui/TouchMainWindow.hpp"
#include <QtWidgets/QApplication>

TouchMainWindow::TouchMainWindow(Settings* settings, QWidget* parent)
  : MainWindow{settings, parent}
{
  messageFrame->setText(tr("To begin, tap the Add Files button. Enter a"
                           " password. Finally, tap the Encrypt or Decrypt"
                           " button."));

  const QSize headerIconSize{45, 45};
  const QSize controlIconSize{50, 50};
  headerFrame->setIconSize(headerIconSize);
  controlButtonFrame->setIconSize(controlIconSize);

  // Adjust stretch of file list view
  contentLayout->setStretch(1, 4);
  // Adjust stretch of message box
  contentLayout->setStretch(2, 1);

  // Load touch-optimized stylesheet
  const auto styleSheet = loadStyleSheet(settings->styleSheetPath(),
                                         "kryvosTouch.qss");

  if (!styleSheet.isEmpty())
  {
    this->setStyleSheet(styleSheet);
  }

  // Connect the password line edit's editing finished signal to the application
  // input method's hide slot. This allows Android's virtual keyboard to close
  // when the user selects Done on the keyboard.
  connect(passwordFrame, &PasswordFrame::editingFinished,
          QGuiApplication::inputMethod(), &QInputMethod::hide);
}

TouchMainWindow::~TouchMainWindow() {}
