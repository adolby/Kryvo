#ifndef KRYVO_WIDGETS_MESSAGEFRAME_HPP_
#define KRYVO_WIDGETS_MESSAGEFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <memory>

namespace Kryvo {

class MessageFramePrivate;

/*!
 * \brief The MessageFrame class contains a text edit display used to inform the
 * user of the status of encryption and decryption operations.
 */
class MessageFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(MessageFrame)
  DECLARE_PRIVATE(MessageFrame)
  std::unique_ptr<MessageFramePrivate> const d_ptr;

 public:
  /*!
   * \brief MessageFrame Constructs a message frame, which is used to inform a
   * user about the status of encryption and decryption operations
   * \param parent Widget parent of this message frame
   */
  explicit MessageFrame(QWidget* parent = nullptr);

  ~MessageFrame();

  /*!
   * \brief appendText Appends text to the message frame
   * \param message Message string
   */
  void appendText(const QString& message);

 public slots:
  /*!
   * \brief pageLeft Switches to the previous message in the message vector
   */
  void pageLeft();

  /*!
   * \brief pageRight Switches to the next message in the message vector.
   */
  void pageRight();
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_MESSAGEFRAME_HPP_
