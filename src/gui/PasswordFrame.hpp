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

#ifndef KRYVOS_GUI_PASSWORDFRAME_HPP_
#define KRYVOS_GUI_PASSWORDFRAME_HPP_

#include <QtWidgets/QFrame>
#include <QtWidgets/QLineEdit>
#include <memory>

class PasswordFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief PasswordFrame
   * \param parent
   */
  explicit PasswordFrame(QWidget* parent = nullptr);
  virtual ~PasswordFrame();

 public:
  /*!
   * \brief passwordLineEdit
   * \return Line edit.
   */
  QLineEdit* passwordLineEdit();

 private:
  class PasswordFramePrivate;
  std::unique_ptr<PasswordFramePrivate> pimpl;
};

#endif // KRYVOS_GUI_PASSWORDFRAME_HPP_