#ifndef KRYVO_WIDGETS_MAINWINDOW_HPP_
#define KRYVO_WIDGETS_MAINWINDOW_HPP_

#include "SettingsFrame.hpp"
#include "HeaderFrame.hpp"
#include "FileListFrame.hpp"
#include "ProgressFrame.hpp"
#include "MessageFrame.hpp"
#include "OutputFrame.hpp"
#include "PasswordFrame.hpp"
#include "ControlButtonFrame.hpp"
#include "settings/Settings.hpp"
#include "utility/pimpl.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <memory>

namespace Kryvo {

class MainWindowPrivate;

/*!
 * \brief The MainWindow class is the main window for the application.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT
  Q_DISABLE_COPY(MainWindow)
  DECLARE_PRIVATE(MainWindow)
  std::unique_ptr<MainWindowPrivate> const d_ptr;

 public:
  /*!
   * \brief MainWindow Constructs the application's main window
   * \param settings Application settings
   * \param parent Widget parent of this main window
   */
  explicit MainWindow(Settings* s = nullptr,
                      QWidget* parent = nullptr);

  /*!
   * \brief ~MainWindow Destroys the application's main window
   */
  ~MainWindow() override;

 signals:
  /*!
   * \brief encrypt Emitted when the user provides all required data for
   * encryption and clicks the Encrypt push button
   * \param passphrase String representing the user supplied passphrase
   * \param inputFiles Input files
   * \param outputPath Output path
   * \param cipher String representing the current cipher
   * \param keySize Key size
   * \param modeOfOperation String representing mode of operation
   * \param compress Boolean representing compression mode
   */
  void encrypt(const QString& cryptoProvider,
               const QString& compressionFormat,
               const QString& passphrase,
               const std::vector<QFileInfo>& inputFiles,
               const QDir& outputPath,
               const QString& cipher,
               std::size_t keySize,
               const QString& modeOfOperation,
               bool removeIntermediateFiles);

  /*!
   * \brief decrypt Emitted when the user provides all required data for
   * decryption and clicks the Decrypt push button
   * \param passphrase String representing the user supplied passphrase
   * \param inputFile Input files
   * \param outputPath Output path
   */
  void decrypt(const QString& passphrase,
               const std::vector<QFileInfo>& inputFiles,
               const QDir& outputPath,
               bool removeIntermediateFiles);

  /*!
   * \brief pause Emitted when the user toggles the Pause push button
   * \param pauseStatus Boolean representing the pause status
   */
  void pause(bool pauseStatus);

  /*!
   * \brief abort Emitted when the user clicks the Clear Files push
   * button
   */
  void abort();

  /*!
   * \brief stopFile Emitted when the user clicks a remove file button
   */
  void stopFile(const QFileInfo& fileInfo);

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
   * \param direction
   */
  void processFiles(Kryvo::CryptDirection cryptDirection);

  /*!
   * \brief updateFileProgress Executed when the cipher operation progress is
   * updated. Updates the progress bar for the item at the specified index.
   * \param fileInfo File path serving as the index to update the progress
   * \param task Task operating on file
   * \param progressValue Integer representing the current progress in percent
   */
  void updateFileProgress(const QFileInfo& fileInfo, const QString& task,
                          qint64 progressValue);

  /*!
   * \brief updateStatusMessage Executed when a message should be displayed to
   * the user. Updates the message text edit text to the message.
   * \param message String containing the message
   */
  void updateStatusMessage(const QString& message);

  /*!
   * \brief updateError Executed when a cipher operation fails
   * \param message String containing the error message
   * \param fileInfo File info
   */
  void updateError(const QString& message, const QFileInfo& fileInfo);

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
  void updateKeySize(std::size_t keySize);

  /*!
   * \brief updateModeOfOperation Executed when the mode of operation is updated
   * by the user in the settings frame
   * \param mode String representing the new mode of operation
   */
  void updateModeOfOperation(const QString& mode);

  /*!
   * \brief updateCompressionFormat Executed when the compression format is
   * updated by the user in the settings frame
   * \param format String representing the new compression format
   */
  void updateCompressionFormat(const QString& format);

  void updateRemoveIntermediateFiles(bool removeIntermediate);

  /*!
   * \brief updateContainerMode Executed when the container mode is updated
   * by the user in the settings frame
   * \param compress Boolean representing the new container mode
   */
  void updateContainerMode(bool container);

  void selectOutputDir();

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
  Settings* settings = nullptr;
  SettingsFrame* settingsFrame = nullptr;
  HeaderFrame* headerFrame = nullptr;
  FileListFrame* fileListFrame = nullptr;
  MessageFrame* messageFrame = nullptr;
  OutputFrame* outputFrame = nullptr;
  PasswordFrame* passwordFrame = nullptr;
  ControlButtonFrame* controlButtonFrame = nullptr;
  QVBoxLayout* contentLayout = nullptr;
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_MAINWINDOW_HPP_
