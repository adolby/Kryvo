/**
 * Kryvos - Encrypts and decrypts files.
 * Copyright (C) 2014, 2015 Andrew Dolby
 *
 * This file is part of Kryvos.
 *
 * Kryvos is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kryvos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kryvos.  If not, see <http://www.gnu.org/licenses/>.
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

/*!
 * \brief The DesktopMainWindow class is used as the main window for the
 * application when a desktop display is the primary display for the operating
 * system.
 */
class DesktopMainWindow : public MainWindow {
 public:
  /*!
   * \brief DesktopMainWindow Constructs the application's main window for
   * desktop displays.
   * \param settings Settings object
   * \param parent Parent widget
   */
  explicit DesktopMainWindow(Settings* settings = nullptr,
                             QWidget* parent = nullptr);

  /*!
   * \brief ~DesktopMainWindow Destroys the application's main window for
   * desktop displays.
   */
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
