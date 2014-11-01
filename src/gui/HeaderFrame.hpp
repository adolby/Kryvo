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

#ifndef KRYVOS_GUI_HEADERFRAME_HPP_
#define KRYVOS_GUI_HEADERFRAME_HPP_

#include <QtWidgets/QFrame>
#include <memory>

/*!
 * \brief The HeaderFrame class contains the header frame which contains the
 * header text, pause button, add files button, and remove all files button.
 */
class HeaderFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief HeaderFrame Constructs a header frame.
   * \param parent Widget parent
   */
  explicit HeaderFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~HeaderFrame Destroys a header frame.
   */
  virtual ~HeaderFrame();

 signals:
  /*!
   * \brief addFiles Emitted when the add files button is clicked.
   */
  void addFiles();

  /*!
   * \brief removeFiles Emitted when the remove files button is clicked.
   */
  void removeFiles();

  /*!
   * \brief pause Emitted when the pause/resume button is checked/unchecked.
   * \param pause The boolean represenitng the pause/resume state of the
   * pause/resume button.
   */
  void pause(bool pause);

  /*!
   * \brief switchFrame Emitted when the settings button is clicked.
   */
  void switchFrame();

 public:
  /*!
   * \brief setIconSize Sets the icon size for buttons.
   * \param iconSize Icon size
   */
  void setIconSize(const QSize& iconSize);

 private slots:
  /*!
   * \brief togglePauseIcon Toggles the pause button icon and text when the
   * paused button is checked.
   */
  void togglePauseIcon(bool toggle);

 private:
  class HeaderFramePrivate;
  std::unique_ptr<HeaderFramePrivate> pimpl;
};

#endif // KRYVOS_GUI_HEADERFRAME_HPP_
