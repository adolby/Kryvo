#ifndef KRYVOS_GUI_OUTPUTFRAME_HPP_
#define KRYVOS_GUI_OUTPUTFRAME_HPP_

#include "src/utility/pimpl.h"
#include <QFrame>

/*!
 * \brief The OutputFrame class contains a line edit that allows the user to
 * enter an output file name.
 */
class OutputFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief OutputFrame Constructs an output frame, which allows a user to enter
   * an output file name
   * \param parent Parent of this output frame
   */
  explicit OutputFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~OutputFrame Destroys an output frame
   */
  virtual ~OutputFrame();

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

 private:
  class OutputFramePrivate;
  pimpl<OutputFramePrivate> m;
};

#endif // KRYVOS_GUI_OUTPUTFRAME_HPP_
