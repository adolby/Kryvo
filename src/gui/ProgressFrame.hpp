#ifndef KRYVOS_GUI_PROGRESSFRAME_HPP_
#define KRYVOS_GUI_PROGRESSFRAME_HPP_

#include "src/utility/pimpl.h"
#include <QFrame>

namespace Kryvos {

inline namespace UI {

/*!
 * \brief The ProgressFrame class contains a label and progress bar for
 * indicating progress about operations not attached to individual files.
 */
class ProgressFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief MessageFrame Constructs a progress frame
   * \param parent Widget parent of this progress frame
   */
  explicit ProgressFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~ProgressFrame Destroys a progress frame
   */
  virtual ~ProgressFrame();

  /*!
   * \brief updateTask Updates the task label
   * \param task New task text
   * \param percentProgress New progress in percent
   */
  void updateTask(const QString& task, const int percentProgress);

 private:
  class ProgressFramePrivate;
  pimpl<ProgressFramePrivate> m;
};

}

}

#endif // KRYVOS_GUI_PROGRESSFRAME_HPP_
