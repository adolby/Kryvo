#ifndef KRYVOS_GUI_MESSAGEFRAME_HPP_
#define KRYVOS_GUI_MESSAGEFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>

/*!
 * \brief The MessageFrame class contains a text edit display used to inform the
 * user of the status of encryption and decryption operations.
 */
class MessageFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief MessageFrame Constructs a message frame, which is used to inform a
   * user about the status of encryption and decryption operations.
   * \param parent Widget parent of this message frame
   */
  explicit MessageFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~MessageFrame Destroys a message frame.
   */
  virtual ~MessageFrame();

  /*!
   * \brief appendText Appends text to the message frame.
   * \param message Message string
   */
  void appendText(const QString& message);

 public slots:
  /*!
   * \brief pageLeft Switches to the previous message in the message vector.
   */
  void pageLeft();

  /*!
   * \brief pageRight Switches to the next message in the message vector.
   */
  void pageRight();

 private:
  class MessageFramePrivate;
  pimpl<MessageFramePrivate> m;
};

#endif // KRYVOS_GUI_MESSAGEFRAME_HPP_
