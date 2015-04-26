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

#ifndef KRYVOS_GUI_PASSWORDFRAME_HPP_
#define KRYVOS_GUI_PASSWORDFRAME_HPP_

#include "utility/pimpl.h"
#include <QtWidgets/QFrame>
#include <QtWidgets/QLineEdit>

/*!
 * \brief The PasswordFrame class contains a line edit that allows the user to
 * enter a password for file encryption/decryption.
 */
class PasswordFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief PasswordFrame Constructs a password frame, which allows a user to
   * enter a password for encrypting/decrypting a file.
   * \param parent The QWidget parent of this password frame.
   */
  explicit PasswordFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~PasswordFrame Destroys a password frame.
   */
  virtual ~PasswordFrame();

 signals:
  /*!
   * \brief editingFinished Emitted when the line edit focus changes or the user
   * indicates they are finished editing with the virtual keyboard.
   */
  void editingFinished();

 public:
  /*!
   * \brief passwordLineEdit Returns the password string from the password line
   * edit.
   * \return Password string.
   */
  QString password() const;

 private:
  class PasswordFramePrivate;
  pimpl<PasswordFramePrivate> m;
};

#endif // KRYVOS_GUI_PASSWORDFRAME_HPP_
