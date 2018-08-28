#ifndef KRYVO_GUI_PASSWORDFRAME_HPP_
#define KRYVO_GUI_PASSWORDFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <memory>

namespace Kryvo {

class PasswordFramePrivate;

/*!
 * \brief The PasswordFrame class contains a line edit that allows the user to
 * enter a password for file encryption/decryption.
 */
class PasswordFrame : public QFrame {
  Q_OBJECT
  DECLARE_PRIVATE(PasswordFrame)
  std::unique_ptr<PasswordFramePrivate> const d_ptr;

 public:
  /*!
   * \brief PasswordFrame Constructs a password frame, which allows a user to
   * enter a password for encrypting/decrypting a file
   * \param parent Parent of this password frame
   */
  explicit PasswordFrame(QWidget* parent = nullptr);

  ~PasswordFrame() override;

  /*!
   * \brief password Returns the password string from the password line
   * edit
   * \return String containing password
   */
  QString password() const;

 signals:
  /*!
   * \brief editingFinished Emitted when the line edit focus changes or the user
   * indicates they are finished editing with the virtual keyboard
   */
  void editingFinished();
};

} // namespace Kryvo

#endif // KRYVO_GUI_PASSWORDFRAME_HPP_
