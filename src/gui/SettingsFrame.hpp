#ifndef KRYVOS_GUI_SETTINGSFRAME_HPP_
#define KRYVOS_GUI_SETTINGSFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <QString>

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
