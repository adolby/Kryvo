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

#ifndef KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_
#define KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_

#include "gui/MainWindow.hpp"
#include "settings/Settings.hpp"
#include <QtGui/QCloseEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QSize>

class DesktopMainWindow : public MainWindow
{
 public:
  explicit DesktopMainWindow(Settings* settings, QWidget* parent = nullptr);
  virtual ~DesktopMainWindow();

 protected:
  /*!
  * \brief closeEvent Executed when the main window is closed.
  * \param event Close event.
  */
  virtual void closeEvent(QCloseEvent* event);

  /*!
  * \brief dragEnterEvent Executed when a drag enter event occurs on the main
  * window.
  * \param event Drag enter event.
  */
  virtual void dragEnterEvent(QDragEnterEvent* event);

  /*!
  * \brief dropEvent Executed when a drop event occurs on the main window.
  * \param event Drop event.
  */
  virtual void dropEvent(QDropEvent* event);

  /*!
   * \brief sizeHint Returns the preferred size of the main window.
   * \return Preferred size.
   */
  virtual QSize sizeHint() const;

  /*!
   * \brief minimumSizeHint Returns the minimum size of the main window.
   * \return Maximum size.
   */
  virtual QSize minimumSizeHint() const;

 private:
  Settings* settings;
};

#endif // KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_
