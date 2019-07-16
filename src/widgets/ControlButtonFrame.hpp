#ifndef KRYVO_WIDGETS_CONTROLBUTTONFRAME_HPP_
#define KRYVO_WIDGETS_CONTROLBUTTONFRAME_HPP_

#include "Constants.hpp"
#include "utility/pimpl.h"
#include <QFrame>
#include <memory>

namespace Kryvo {

class ControlButtonFramePrivate;

/*!
 * \brief The ControlButtonFrame class contains the encryption/decryption
 * buttons.
 */
class ControlButtonFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(ControlButtonFrame)
  DECLARE_PRIVATE(ControlButtonFrame)
  std::unique_ptr<ControlButtonFramePrivate> const d_ptr;

 public:
  /*!
   * \brief ControlButtonFrame Constructs a control button frame.
   * \param parent Widget parent of this control button frame
   */
  explicit ControlButtonFrame(QWidget* parent = nullptr);

  ~ControlButtonFrame() override;

  /*!
   * \brief setIconSize Sets the icon size for buttons.
   * \param iconSize Icon size
   */
  void setIconSize(const QSize& iconSize);

 signals:
  /*!
   * \brief processFiles Emitted when the encrypt or decrypt push buttons are
   * clicked.
   * \param direction
   */
  void processFiles(Kryvo::CryptDirection direction);

 private slots:
  /*!
   * \brief encryptFiles Executed when the encrypt push button is clicked.
   */
  void encryptFiles();

  /*!
   * \brief decryptFiles Executed when the decrypt push button is clicked.
   */
  void decryptFiles();
};

}

#endif // KRYVO_WIDGETS_CONTROLBUTTONFRAME_HPP_
