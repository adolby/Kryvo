#ifndef KRYVO_UI_UI_HPP_
#define KRYVO_UI_UI_HPP_

#include "settings/Settings.hpp"
#include "cryptography/EncryptConfig.hpp"
#include "cryptography/DecryptConfig.hpp"
#include "utility/pimpl.h"
#include "Constants.hpp"
#include <QFileInfo>
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <memory>

namespace Kryvo {

class UiPrivate;

/*!
 * \brief The Ui class is the UI for the application.
 */
class Ui : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(Ui)

  Q_PROPERTY(QUrl inputPath READ inputPath NOTIFY inputPathChanged)
  Q_PROPERTY(QUrl outputPath READ outputPath NOTIFY outputPathChanged)
  Q_PROPERTY(QString outputPathString READ outputPathString
             NOTIFY outputPathChanged)
  Q_PROPERTY(QString cryptoProvider READ cryptoProvider
             NOTIFY cryptoProviderChanged)
  Q_PROPERTY(QString keySize READ keySize NOTIFY keySizeChanged)
  Q_PROPERTY(QString modeOfOperation READ modeOfOperation
             NOTIFY modeOfOperationChanged)
  Q_PROPERTY(QString compressionFormat READ compressionFormat
             NOTIFY compressionFormatChanged)
  Q_PROPERTY(bool removeIntermediateFiles READ removeIntermediateFiles
             NOTIFY removeIntermediateFilesChanged)
  Q_PROPERTY(bool containerMode READ containerMode NOTIFY containerModeChanged)
  Q_PROPERTY(QString statusMessage READ statusMessage
             NOTIFY statusMessageChanged)
  Q_PROPERTY(QString password READ password NOTIFY passwordChanged)

  DECLARE_PRIVATE(Ui)

  std::unique_ptr<UiPrivate> const d_ptr;

 public:
  /*!
   * \brief Ui Constructs the application UI
   * \param settings Application settings
   * \param parent Parent of this UI object
   */
  explicit Ui(Settings* s = nullptr, QObject* parent = nullptr);

  /*!
   * \brief ~Ui Destroys the application UI
   */
  ~Ui() override;

  // UI navigation data
  QVariantMap currentPage() const;
  QVariantMap page(int index) const;
  bool canNavigateBack() const;

  // Encrypt/decrypt data
  QUrl inputPath() const;
  QUrl outputPath() const;
  QString outputPathString() const;
  QString password() const;

  // Settings data
  QString cryptoProvider() const;
  QString keySize() const;
  QString modeOfOperation() const;
  QString compressionFormat() const;
  bool removeIntermediateFiles() const;
  bool containerMode() const;

  // Message navigation data
  QString statusMessage() const;

 signals:
  /*!
   * \brief encrypt Emitted when the user provides all required data for
   * encryption and clicks the Encrypt button
   * \param config Encrypt config
   * \param inputFiles Files to encrypt
   * \param outputDir Output dir
   */
  void encrypt(const Kryvo::EncryptConfig& config,
               const std::vector<QFileInfo>& inputFiles,
               const QDir& outputDir);

  /*!
   * \brief decrypt Emitted when the user provides all required data for
   * decryption and clicks the Decrypt button
   * \param config Decrypt config
   * \param inputFiles Files to decrypt
   * \param outputDir Output dir
   */
  void decrypt(const Kryvo::DecryptConfig& config,
               const std::vector<QFileInfo>& inputFiles,
               const QDir& outputDir);

  /*!
   * \brief pause Emitted when the user toggles the Pause push button
   * \param pauseStatus Boolean representing the pause status
   */
  void pause(bool pauseStatus);

  /*!
   * \brief cancel Emitted when the user clicks the Clear Files push
   * button
   */
  void cancel();

  /*!
   * \brief stopFile Emitted when the user clicks a remove file button
   */
  void stopFile(const QFileInfo& fileInfo);

  void quitApp();

  void pushPage(const QVariantMap& page);
  void popPage();

  // Request settings update
  void requestUpdateCryptoProvider(const QString& cipher);
  void requestUpdateKeySize(std::size_t size);
  void requestUpdateModeOfOperation(const QString& modeOfOperation);
  void requestUpdateCompressionFormat(const QString& format);
  void requestUpdateRemoveIntermediateFiles(bool removeIntermediateFiles);
  void requestUpdateContainerMode(bool containerMode);
  void requestUpdateInputPath(const QString& path);
  void requestUpdateOutputPath(const QString& path);

  // Receive settings update notification
  void cryptoProviderChanged(const QString& cipher);
  void keySizeChanged(const QString& keySize);
  void modeOfOperationChanged(const QString& modeOfOperation);
  void compressionFormatChanged(const QString& format);
  void removeIntermediateFilesChanged(bool removeIntermediateFiles);
  void containerModeChanged(bool containerMode);
  void inputPathChanged(const QUrl& url);
  void outputPathChanged(const QUrl& url);

  void statusMessageChanged(const QString& message);
  void passwordChanged(const QString& password);

 public slots:
  void init();

  void changePage(const QString& name);

  void changePage(const QString& name, const QVariantMap& properties);

  void changePage(const QString& name, const QVariantMap& properties,
                  int delayInMSecs);

  void navigate(const QString& name,
                const QVariantMap& properties = QVariantMap());

  void navigateBack();

  void clearNavigationHistory();

  void addFile(const QUrl& fileUrl);

  /*!
   * \brief addFiles Executed when the Add Files toolbar button is clicked
   */
  void addFiles(const QList<QUrl>& fileUrls);

  /*!
   * \brief removeFiles Executed when the Remove All Files toolbar button is
   * clicked
   */
  void removeFiles();

  void encryptFiles(const QString& passphrase);

  void decryptFiles(const QString& passphrase);

  /*!
   * \brief processFiles Executed when the encrypt or decrypt button is
   * clicked. Starts the encryption or decryption operation using the passphrase
   * from the password line edit, the file list from the file list model, and
   * the algorithm name from the settings panel.
   *
   * \param outputPath String containing output path
   * \param passphrase String containing passphrase
   * \param cryptDirection Boolean representing encrypt or decrypt
   */
  void processFiles(const QString& passphrase,
                    Kryvo::CryptDirection cryptDirection);

  /*!
   * \brief pauseProcessing Emitted when the user toggles the Pause push button
   * \param pauseStatus Boolean representing the pause status
   */
  void pauseProcessing(bool pauseStatus);

  /*!
   * \brief updateFileProgress Executed when the cipher operation progress is
   * updated. Updates the progress bar for the item at the specified index.
   * \param filePath File path serving as the index to update the progress
   * \param task Task operating on file
   * \param progressValue Integer representing the current progress in percent
   */
  void updateFileProgress(const QFileInfo& filePath, const QString& task,
                          qint64 progressValue);

  /*!
   * \brief appendStatusMessage Executed when a message should be displayed to
   * the user. Updates the current message and adds to the message list.
   * \param message String containing the message
   */
  void appendStatusMessage(const QString& message);

  /*
   * \brief appendErrorMessage Executed when an error occurs
   * \param message String containing the error message
   * \param path String containing the error file name path
   */
  void appendErrorMessage(const QString& message, const QFileInfo& fileName);

  /*!
   * \brief updateCryptoProvider Executed when the crypto provider is updated by
   * the user in the settings frame
   * \param provider String representing the new crypto provider
   */
  void updateCryptoProvider(const QString& provider);

  /*!
   * \brief updateKeySize Executed when the key size is updated by the user in
   * the settings frame
   * \param keySize String representing key size in bits
   */
  void updateKeySize(const QString& keySizeString);

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
   * \brief updateContainerMode Executed when the container mode is updated by
   * the user in the settings frame
   * \param compress Boolean representing the new container mode
   */
  void updateContainerMode(bool container);

  void updateOutputPath(const QUrl& url);

  void navigateMessageLeft();

  void navigateMessageRight();

  void updatePassword(const QString& password);
};

} // namespace Kryvo

#endif // KRYVO_UI_UI_HPP_
