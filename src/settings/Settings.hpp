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

#ifndef KRYVOS_SETTINGS_SETTINGS_HPP_
#define KRYVOS_SETTINGS_SETTINGS_HPP_

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <memory>

class Settings {
 public:
  /*!
   * \brief Settings Constructs an instance of the settings management class.
   */
  explicit Settings();

  /*!
   * \brief ~Settings Destroys an instance of the settings management class.
   */
  virtual ~Settings();

  /*!
   * \brief position Sets the main window position.
   * \param position Point at position of the main window.
   */
  void position(const QPoint& position);

  /*!
   * \brief position Returns the main window's position.
   * \return Point at position of the main window.
   */
  QPoint position() const;

  /*!
   * \brief maximized Sets the maximized state.
   * \param maximized Boolean representing maximized state.
   */
  void maximized(bool maximized);

  /*!
   * \brief maximized Returns the main window's maximized state.
   * \return Boolean representing maximized state.
   */
  bool maximized() const;

  /*!
   * \brief size Sets the main window size.
   * \param size Size of the main window.
   */
  void size(const QSize& size);

  /*!
   * \brief size Returns the main window's last size.
   * \return Size of main window.
   */
  QSize size() const;

  /*!
   * \brief lastAlgorithm Sets the last used algorithm for later storage.
   * \param algorithm String containing the algorithm name.
   */
  void lastAlgorithm(const QString& algorithmName);

  /*!
   * \brief lastAlgorithm Returns the name of the last used algorithm.
   * \return String containing the algorithm name.
   */
  QString lastAlgorithm() const;

  /*!
   * \brief lastDirectory Sets the last opened directory for later storage.
   * \param algorithm String containing the directory path.
   */
  void lastDirectory(const QString& directory);

  /*!
   * \brief lastDirectory Returns the name of the last opened directory.
   * \return String containing the directory path.
   */
  QString lastDirectory() const;

  /*!
   * \brief styleSheetPath Returns the path to the application stylesheet.
   * \return String containing the stylesheet path.
   */
  QString styleSheetPath() const;

 private:
  class SettingsPrivate;
  std::unique_ptr<SettingsPrivate> pimpl;
};

#endif // KRYVOS_SETTINGS_SETTINGS_HPP_
