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

#ifndef KRYVOS_GUI_MAINWINDOW_HPP_
#define KRYVOS_GUI_MAINWINDOW_HPP_

#include "settings/Settings.hpp"
#include "gui/SettingsFrame.hpp"
#include "gui/HeaderFrame.hpp"
#include "gui/FileListFrame.hpp"
#include "gui/MessageFrame.hpp"
#include "gui/PasswordFrame.hpp"
#include "gui/ControlButtonFrame.hpp"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <memory>

/*!
 * \brief The MainWindow class is the main window for the application.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  /*!
   * \brief MainWindow Constructs the application's main window.
   * \param parent The QWidget parent of this main window.
   */
  explicit MainWindow(Settings* settings = nullptr, QWidget* parent = nullptr);

  /*!
   * \brief ~MainWindow Destroys the application's main window.
   */
  virtual ~MainWindow();

 signals:
  /*!
   * \brief encrypt Emitted when the user provides all required data for
   * encryption and clicks the Encrypt push button.
   * \param passphrase String representing the user supplied passphrase.
   * \param inputFileNames List of input file strings.
   * \param algorithmName String representing the current algorithm.
   * Example: "AES-128/GCM" will encrypt files with AES 128-bit key size in
   * Gallois Counter Mode.
   */
  void encrypt(const QString& passphrase,
               const QStringList& inputFileNames,
               const QString& algorithmName);

  /*!
   * \brief decrypt Emitted when the user provides all required data for
   * decryption and clicks the Decrypt push button.
   * \param passphrase String representing the user supplied passphrase.
   * \param inputFileNames The list of input file strings.
   */
  void decrypt(const QString& passphrase, const QStringList& inputFileNames);

  /*!
   * \brief pauseCipher Emitted when the user toggles the Pause push button.
   * \param pause The boolean representing the pause status.
   */
  void pauseCipher(bool pause);

  /*!
   * \brief stopCipher Emitted when the user clicks the Clear Files push button.
   */
  void abortCipher();

  /*!
   * \brief stopFile Emitted when the user clicks a remove file button.
   */
  void stopFile(const QString& fileName);

 public slots:
  /*!
   * \brief addFiles Executed when the Add Files toolbar push button is clicked.
   */
  void addFiles();

  /*!
   * \brief removeFiles Executed when the Remove All Files toolbar push
   * button is clicked.
   */
  void removeFiles();

  /*!
   * \brief processFiles Executed when the encrypt or decrypt push button is
   * clicked. Starts the encryption or decryption operation using the passphrase
   * from the password line edit, the file list from the file list model, and
   * the algorithm name from the settings panel.
   * \param cryptFlag Boolean representing encrypt (true) or decrypt (false).
   */
  void processFiles(bool cryptFlag);

  /*!
   * \brief updateProgress Executed when the cipher operation progress is
   * updated. Updates the progress bar for the item at the specified index.
   * \param index Integer representing the file list index to update.
   * \param percent Integer representing the current progress in percent.
   */
  void updateProgress(const QString& path, qint64 percent);

  /*!
   * \brief updateStatusMessage Executed when a message should be displayed to
   * the user. Updates the message text edit text to the message.
   * \param message String representing the message.
   */
  void updateStatusMessage(const QString& message);

  /*!
   * \brief updateError Executed when a cipher operation fails.
   * \param index Integer representing the file list index to update.
   * \param message String representing the error message.
   */
  void updateError(const QString& path, const QString& message);

  /*!
   * \brief updateBusyStatus Executed when the cipher operation updates its busy
   * status. Stores the status to allow the GUI to decide when the user can
   * request new encryption events.
   * \param busy Boolean representing the busy status.
   */
  void updateBusyStatus(bool busy);

 protected:
  /*!
   * \brief loadStyleSheet Attempts to load a Qt stylesheet from the local
   * themes folder with the name specified in the local settings file. If the
   * load fails, the method will load the default stylesheet from the
   * application resources.
   * \param styleFile String representing the name of the stylesheet without
   * a file extension.
   * \param defaultFile String containing the name of the default stylesheet,
   * which will be used if the selected stylesheet file doesn't exist.
   * \return String containing the stylesheet file contents.
   */
  QString loadStyleSheet(const QString& styleFile, const QString& defaultFile);

 protected:
  SettingsFrame* settingsFrame;
  HeaderFrame* headerFrame;
  FileListFrame* fileListFrame;
  MessageFrame* messageFrame;
  PasswordFrame* passwordFrame;
  ControlButtonFrame* controlButtonFrame;
  QVBoxLayout* contentLayout;

 private:
  class MainWindowPrivate;
  std::unique_ptr<MainWindowPrivate> pimpl;
};

#endif // KRYVOS_GUI_MAINWINDOW_HPP_
