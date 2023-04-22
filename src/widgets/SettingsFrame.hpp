#ifndef KRYVO_WIDGETS_SETTINGSFRAME_HPP_
#define KRYVO_WIDGETS_SETTINGSFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <QString>
#include <memory>

namespace Kryvo {

class SettingsFramePrivate;

/*!
 * \brief The SettingsFrame class contains controls for customizing encryption
 * settings.
 */
class SettingsFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(SettingsFrame)
  DECLARE_PRIVATE(SettingsFrame)
  std::unique_ptr<SettingsFramePrivate> const d_ptr;

 public:
  /*!
   * \brief SettingsFrame Constructs a settings frame
   * \param cryptoProvider String representing the crypto provider name
   * \param compressionFormat String representing compression format
   * \param keySize String representing the key size
   * \param removeIntermediateFiles Remove intermediate files enable/disable
   * \param containerMode Container mode archives multiple input files together
   * \param parent QWidget parent
   */
  explicit SettingsFrame(QWidget* parent = nullptr);

  ~SettingsFrame() override;

 signals:
  /*!
   * \brief switchFrame Emitted when the user requests that the main frame
   * should be displayed
   */
  void switchFrame();

  /*!
   * \brief requestUpdateCryptoProvider Emitted when the user has changed the
   * crypto provider via the combobox representing it
   * \param provider String containing the provider name
   */
  void requestUpdateCryptoProvider(const QString& provider);

  /*!
   * \brief requestUpdateKeySize Emitted when the user has changed the key size
   * via the combobox representing it
   * \param keySize Key size in bits
   */
  void requestUpdateKeySize(std::size_t keySize);

  /*!
   * \brief requestUpdateCompressionMode Emitted when the user has changed the
   * compression mode via the checkbox representing it
   * \param format String representing compression format
   */
  void requestUpdateCompressionFormat(const QString& format);

  /*!
   * \brief requestUpdateRemoveIntermediateFiles Emitted when the user has
   * changed the remove intermediate files preference via the checkbox
   * representing it
   * \param compress Boolean representing remove intermediate files preference
   */
  void requestUpdateRemoveIntermediateFiles(bool removeIntermediate);

  /*!
   * \brief requestUpdateContainerMode Emitted when the user has changed the
   * container mode via the checkbox representing it
   * \param container Boolean representing container
   */
  void requestUpdateContainerMode(bool container);

public:
  void init(const QString& cryptoProvider,
            const QString& compressionFormat,
            std::size_t keySize,
            bool removeIntermediateFiles,
            bool containerMode);

 public slots:
  /*!
   * \brief updateCryptoProvider Executed when the crypto provider changes
   */
  void updateCryptoProvider();

  /*!
   * \brief updateKeySize Executed when the key size changes
   */
  void updateKeySize();

  /*!
   * \brief updateCompressionFormat Executed when the compression format changes
   */
  void updateCompressionFormat();

  /*!
   * \brief updateRemoveIntermediateFiles Executed when the remove intermediate
   * files preference changes
   */
  void updateRemoveIntermediateFiles();

  /*!
   * \brief changeContainerMode Executed when the container mode changes
   */
  void updateContainerMode();

  void cryptoProviderChanged(const QString& provider);
  void keySizeChanged(std::size_t keySize);
  void compressionFormatChanged(const QString& format);
  void removeIntermediateFilesChanged(bool removeIntermediate);
  void containerModeChanged(bool container);
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_SETTINGSFRAME_HPP_
