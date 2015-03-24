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

#ifndef KRYVOS_GUI_CONTROLBUTTONFRAME_HPP_
#define KRYVOS_GUI_CONTROLBUTTONFRAME_HPP_

#include <QtWidgets/QFrame>
#include <memory>

/*!
 * \brief The ControlButtonFrame class contains the encryption/decryption
 * buttons.
 */
class ControlButtonFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief ControlButtonFrame Constructs a control button frame.
   * \param parent Widget parent of this control button frame
   */
  explicit ControlButtonFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~ControlButtonFrame Destroys a control button frame.
   */
  virtual ~ControlButtonFrame();

 signals:
  /*!
   * \brief processFiles Emitted when the encrypt or decrypt push buttons are
   * clicked.
   * \param cryptFlag Boolean representing encrypt (true) or decrypt (false)
   */
  void processFiles(const bool cryptFlag);

 private slots:
  /*!
   * \brief encryptFiles Executed when the encrypt push button is clicked.
   */
  void encryptFiles();

  /*!
   * \brief decryptFiles Executed when the decrypt push button is clicked.
   */
  void decryptFiles();

 public:
  /*!
   * \brief setIconSize Sets the icon size for buttons.
   * \param iconSize Icon size
   */
  void setIconSize(const QSize& iconSize);

 private:
  class ControlButtonFramePrivate;
  std::unique_ptr<ControlButtonFramePrivate> pimpl;
};

#endif // KRYVOS_GUI_CONTROLBUTTONFRAME_HPP_
