#ifndef KRYVOS_GUI_PASSWORDFRAME_HPP_
#define KRYVOS_GUI_PASSWORDFRAME_HPP_

#include "src/utility/pimpl.h"
#include <QFrame>

/*!
 * \brief The PasswordFrame class contains a line edit that allows the user to
 * enter a password for file encryption/decryption.
 */
class PasswordFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief PasswordFrame Constructs a password frame, which allows a user to
   * enter a password for encrypting/decrypting a file
   * \param parent Parent of this password frame
   */
  explicit PasswordFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~PasswordFrame Destroys a password frame
   */
  virtual ~PasswordFrame();

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

 private:
  class PasswordFramePrivate;
  pimpl<PasswordFramePrivate> m;
};

#endif // KRYVOS_GUI_PASSWORDFRAME_HPP_
