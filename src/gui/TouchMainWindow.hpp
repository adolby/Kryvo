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

#ifndef KRYVOS_GUI_TOUCHMAINWINDOW_HPP_
#define KRYVOS_GUI_TOUCHMAINWINDOW_HPP_

#include "gui/MainWindow.hpp"
#include "settings/Settings.hpp"

/*!
 * \brief The TouchMainWindow class is used as the main window for the
 * application when a touch-based display is the primary display for the
 * operating system.
 */
class TouchMainWindow : public MainWindow
{
 public:
  /*!
   * \brief TouchMainWindow Constructs the application's main window for
   * touch-based displays.
   * \param settings Settings object
   * \param parent Widget parent
   */
  TouchMainWindow(Settings* settings = nullptr, QWidget* parent = nullptr);

  /*!
   * \brief ~TouchMainWindow Destroys the application's main window for
   * touch-based displays.
   */
  ~TouchMainWindow();
};

#endif // KRYVOS_GUI_TOUCHMAINWINDOW_HPP_
