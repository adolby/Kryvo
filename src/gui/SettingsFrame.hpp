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

#ifndef KRYVOS_GUI_SETTINGSFRAME_HPP_
#define KRYVOS_GUI_SETTINGSFRAME_HPP_

#include "utility/pimpl.h"
#include <QtWidgets/QFrame>
#include <QtCore/QString>

/*!
 * \brief The SettingsFrame class contains controls for customizing encryption
 * settings.
 */
class SettingsFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief SettingsFrame Constructs a settings frame.
   * \param cipher String representing the cipher name
   * \param keySize String representing the key size
   * \param mode String representing the mode of operation
   * \param parent QWidget parent
   */
  explicit SettingsFrame(const QString& cipher,
                         const std::size_t& keySize,
                         const QString& mode,
                         QWidget* parent = nullptr);

  /*!
   * \brief ~SettingsFrame Destroys a settings frame.
   */
  virtual ~SettingsFrame();

 signals:
  /*!
   * \brief switchFrame Emitted when the user requests that the main frame
   * should be displayed.
   */
  void switchFrame();

  /*!
   * \brief newCipher Emitted when the user has changed the cipher algorithm via
   * the combo box representing it.
   * \param newCipher String containing the new cipher name
   */
  void newCipher(const QString& cipher);

  /*!
   * \brief newKeySize Emitted when the user has changed the key size via the
   * combo box representing it.
   * \param keySize Key size in bits
   */
  void newKeySize(const std::size_t& keySize);

  /*!
   * \brief newModeOfOperation Emitted when the user has changed the cipher
   * mode of operation via the combo box representing it.
   * \param modeOfOperation String containing the new mode of operation
   */
  void newModeOfOperation(const QString& modeOfOperation);

 public slots:
  /*!
   * \brief changeCipher Executed when the cipher changes.
   */
  void changeCipher();

  /*!
   * \brief changeKeySize Executed when the key size changes.
   */
  void changeKeySize();

  /*!
   * \brief changeModeOfOperation Executed when the mode of operation changes.
   */
  void changeModeOfOperation();

 private:
  class SettingsFramePrivate;
  pimpl<SettingsFramePrivate> m;
};

#endif // KRYVOS_GUI_SETTINGSFRAME_HPP_
