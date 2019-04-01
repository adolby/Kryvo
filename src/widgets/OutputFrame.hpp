#ifndef KRYVO_WIDGETS_OUTPUTFRAME_HPP_
#define KRYVO_WIDGETS_OUTPUTFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <memory>

namespace Kryvo {

class OutputFramePrivate;

/*!
 * \brief The OutputFrame class contains a line edit that allows the user to
 * enter an output file name.
 */
class OutputFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(OutputFrame)
  DECLARE_PRIVATE(OutputFrame)
  std::unique_ptr<OutputFramePrivate> const d_ptr;

 public:
  /*!
   * \brief OutputFrame Constructs an output frame, which allows a user to enter
   * an output file name
   * \param parent Parent of this output frame
   */
  explicit OutputFrame(QWidget* parent = nullptr);

  ~OutputFrame() override;

  /*!
   * \brief outputPath Updates the output file path string of the output line
   * edit
   * \param String containing output file path
   */
  void outputPath(const QString& path);

  /*!
   * \brief outputPath Returns the output file path string from the output line
   * edit
   * \return String containing output file path
   */
  QString outputPath() const;

 signals:
  /*!
   * \brief editingFinished Emitted when the line edit focus changes or the user
   * indicates they are finished editing with the virtual keyboard
   */
  void editingFinished();
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_OUTPUTFRAME_HPP_
