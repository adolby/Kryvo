#ifndef KRYVOS_GUI_SETTINGSFRAME_HPP_
#define KRYVOS_GUI_SETTINGSFRAME_HPP_

#include "src/utility/pimpl.h"
#include <QFrame>
#include <QString>

namespace Kryvos {

inline namespace UI {

/*!
 * \brief The SettingsFrame class contains controls for customizing encryption
 * settings.
 */
class SettingsFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief SettingsFrame Constructs a settings frame
   * \param cipher String representing the cipher name
   * \param keySize String representing the key size
   * \param mode String representing the mode of operation
   * \param parent QWidget parent
   */
  explicit SettingsFrame(const QString& cipher,
                         const std::size_t keySize,
                         const QString& mode,
                         const bool compressionMode,
                         const bool containerMode,
                         QWidget* parent = nullptr);

  /*!
   * \brief ~SettingsFrame Destroys a settings frame.
   */
  virtual ~SettingsFrame();

 signals:
  /*!
   * \brief switchFrame Emitted when the user requests that the main frame
   * should be displayed
   */
  void switchFrame();

  /*!
   * \brief updateCipher Emitted when the user has changed the cipher algorithm
   * via the combobox representing it
   * \param cipher String containing the cipher name
   */
  void updateCipher(const QString& cipher);

  /*!
   * \brief updateKeySize Emitted when the user has changed the key size via the
   * combobox representing it
   * \param keySize Key size in bits
   */
  void updateKeySize(const std::size_t keySize);

  /*!
   * \brief updateModeOfOperation Emitted when the user has changed the cipher
   * mode of operation via the combobox representing it
   * \param modeOfOperation String containing the mode of operation
   */
  void updateModeOfOperation(const QString& modeOfOperation);

  /*!
   * \brief updateCompressionMode Emitted when the user has changed the
   * compression mode via the checkbox representing it
   * \param compress Boolean representing compression
   */
  void updateCompressionMode(const bool compress);

  /*!
   * \brief updateContainerMode Emitted when the user has changed the container
   * mode via the checkbox representing it
   * \param container Boolean representing container
   */
  void updateContainerMode(const bool container);

 public slots:
  /*!
   * \brief changeCipher Executed when the cipher changes
   */
  void changeCipher();

  /*!
   * \brief changeKeySize Executed when the key size changes
   */
  void changeKeySize();

  /*!
   * \brief changeModeOfOperation Executed when the mode of operation changes
   */
  void changeModeOfOperation();

  /*!
   * \brief changeCompressionMode Executed when the compression mode changes
   */
  void changeCompressionMode();

  /*!
   * \brief changeContainerMode Executed when the container mode changes
   */
  void changeContainerMode();

 private:
  class SettingsFramePrivate;
  pimpl<SettingsFramePrivate> m;
};

}

}

#endif // KRYVOS_GUI_SETTINGSFRAME_HPP_
