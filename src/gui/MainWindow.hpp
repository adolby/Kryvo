#ifndef KRYVOS_GUI_MAINWINDOW_HPP_
#define KRYVOS_GUI_MAINWINDOW_HPP_

#include "src/settings/Settings.hpp"
#include "src/gui/SettingsFrame.hpp"
#include "src/gui/HeaderFrame.hpp"
#include "src/gui/FileListFrame.hpp"
#include "src/gui/MessageFrame.hpp"
#include "src/gui/PasswordFrame.hpp"
#include "src/gui/ControlButtonFrame.hpp"
#include "src/utility/pimpl.h"
#include <QMainWindow>
#include <QVBoxLayout>

/*!
 * \brief The MainWindow class is the main window for the application.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  /*!
   * \brief MainWindow Constructs the application's main window
   * \param settings Application settings
   * \param parent Widget parent of this main window
   */
  explicit MainWindow(Settings* settings = nullptr, QWidget* parent = nullptr);

  /*!
   * \brief ~MainWindow Destroys the application's main window
   */
  virtual ~MainWindow();

 signals:
  /*!
   * \brief encrypt Emitted when the user provides all required data for
   * encryption and clicks the Encrypt push button
   * \param passphrase String representing the user supplied passphrase
   * \param inputFileNames List of input file strings
   * \param cipher String representing the current cipher
   * \param keySize Key size
   * \param modeOfOperation String representing mode of operation
   * \param compress Boolean representing compression mode
   */
  void encrypt(const QString& passphrase,
               const QStringList& inputFileNames,
               const QString& cipher,
               const std::size_t& keySize,
               const QString& modeOfOperation,
               const bool compress);

  /*!
   * \brief decrypt Emitted when the user provides all required data for
   * decryption and clicks the Decrypt push button
   * \param passphrase String representing the user supplied passphrase
   * \param inputFileNames List of input file strings
   */
  void decrypt(const QString& passphrase, const QStringList& inputFileNames);

  /*!
   * \brief pauseCipher Emitted when the user toggles the Pause push button
   * \param pause Boolean representing the pause status
   */
  void pauseCipher(const bool pause);

  /*!
   * \brief abortCipher Emitted when the user clicks the Clear Files push
   * button
   */
  void abortCipher();

  /*!
   * \brief stopFile Emitted when the user clicks a remove file button
   */
  void stopFile(const QString& fileName);

 public slots:
  /*!
   * \brief addFiles Executed when the Add Files toolbar push button is clicked
   */
  void addFiles();

  /*!
   * \brief removeFiles Executed when the Remove All Files toolbar push
   * button is clicked
   */
  void removeFiles();

  /*!
   * \brief processFiles Executed when the encrypt or decrypt push button is
   * clicked. Starts the encryption or decryption operation using the passphrase
   * from the password line edit, the file list from the file list model, and
   * the algorithm name from the settings panel.
   * \param cryptFlag Boolean representing encrypt (true) or decrypt (false)
   */
  void processFiles(const bool cryptFlag);

  /*!
   * \brief updateProgress Executed when the cipher operation progress is
   * updated. Updates the progress bar for the item at the specified index.
   * \param index Integer representing the file list index to update
   * \param percent Integer representing the current progress in percent
   */
  void updateProgress(const QString& path, const qint64 percent);

  /*!
   * \brief updateStatusMessage Executed when a message should be displayed to
   * the user. Updates the message text edit text to the message.
   * \param message String representing the message.
   */
  void updateStatusMessage(const QString& message);

  /*!
   * \brief updateError Executed when a cipher operation fails
   * \param index Integer representing the file list index to update
   * \param message String representing the error message
   */
  void updateError(const QString& path, const QString& message);

  /*!
   * \brief updateBusyStatus Executed when the cipher operation updates its busy
   * status. Stores the status to allow the GUI to decide when the user can
   * request new encryption.
   * \param busy Boolean representing the busy status
   */
  void updateBusyStatus(const bool busy);

  /*!
   * \brief updateCipher Executed when the cipher is updated by the user in the
   * settings frame
   * \param cipher String representing the new cipher
   */
  void updateCipher(const QString& cipher);

  /*!
   * \brief updateKeySize Executed when the key size is updated by the user in
   * the settings frame
   * \param keySize Key size in bits
   */
  void updateKeySize(const std::size_t& keySize);

  /*!
   * \brief updateModeOfOperation Executed when the mode of operation is updated
   * by the user in the settings frame
   * \param mode String representing the new mode of operation
   */
  void updateModeOfOperation(const QString& mode);

  /*!
   * \brief updateCompressionMode Executed when the compression mode is updated
   * by the user in the settings frame
   * \param compress Boolean representing the new compression mode
   */
  void updateCompressionMode(const bool compress);

  /*!
   * \brief updateContainerMode Executed when the container mode is updated
   * by the user in the settings frame
   * \param compress Boolean representing the new container mode
   */
  void updateContainerMode(const bool container);

 protected:
  /*!
   * \brief loadStyleSheet Attempts to load a Qt stylesheet from the local
   * themes folder with the name specified in the local settings file. If the
   * load fails, the method will load the default stylesheet from the
   * application resources.
   * \param styleFile String representing the name of the stylesheet without
   * a file extension
   * \param defaultFile String containing the name of the default stylesheet,
   * which will be used if the selected stylesheet file doesn't exist
   * \return String containing the stylesheet file contents
   */
  QString loadStyleSheet(const QString& styleFile,
                         const QString& defaultFile) const;

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
  pimpl<MainWindowPrivate> m;
};

#endif // KRYVOS_GUI_MAINWINDOW_HPP_
