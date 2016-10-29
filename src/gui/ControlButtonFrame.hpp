#ifndef KRYVOS_GUI_CONTROLBUTTONFRAME_HPP_
#define KRYVOS_GUI_CONTROLBUTTONFRAME_HPP_

#include "src/utility/pimpl.h"
#include <QFrame>

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

  /*!
   * \brief setIconSize Sets the icon size for buttons.
   * \param iconSize Icon size
   */
  void setIconSize(const QSize& iconSize);

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

 private:
  class ControlButtonFramePrivate;
  pimpl<ControlButtonFramePrivate> m;
};

#endif // KRYVOS_GUI_CONTROLBUTTONFRAME_HPP_
