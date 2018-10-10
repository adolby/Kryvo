#ifndef KRYVO_GUI_PROGRESSFRAME_HPP_
#define KRYVO_GUI_PROGRESSFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <memory>

namespace Kryvo {

class ProgressFramePrivate;

/*!
 * \brief The ProgressFrame class contains a label and progress bar for
 * indicating progress about operations not attached to individual files.
 */
class ProgressFrame : public QFrame {
  Q_OBJECT
  DECLARE_PRIVATE(ProgressFrame)
  std::unique_ptr<ProgressFramePrivate> const d_ptr;

 public:
  /*!
   * \brief MessageFrame Constructs a progress frame
   * \param parent Widget parent of this progress frame
   */
  explicit ProgressFrame(QWidget* parent = nullptr);

  ~ProgressFrame() override;

  /*!
   * \brief updateTask Updates the task label
   * \param task New task text
   * \param percentProgress New progress in percent
   */
  void updateTask(const QString& task, int percentProgress);
};

} // namespace Kryvo

#endif // KRYVO_GUI_PROGRESSFRAME_HPP_
