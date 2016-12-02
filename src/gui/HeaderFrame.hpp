#ifndef KRYVOS_GUI_HEADERFRAME_HPP_
#define KRYVOS_GUI_HEADERFRAME_HPP_

#include "src/utility/pimpl.h"
#include <QFrame>

/*!
 * \brief The HeaderFrame class contains the header frame which contains the
 * header text, pause button, add files button, and remove all files button.
 */
class HeaderFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief HeaderFrame Constructs a header frame
   * \param parent Widget parent
   */
  explicit HeaderFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~HeaderFrame Destroys a header frame
   */
  virtual ~HeaderFrame();

  /*!
   * \brief setIconSize Sets the icon size for buttons
   * \param iconSize Icon size
   */
  void setIconSize(const QSize& iconSize);

 signals:
  /*!
   * \brief addFiles Emitted when the add files button is clicked
   */
  void addFiles();

  /*!
   * \brief removeFiles Emitted when the remove files button is clicked
   */
  void removeFiles();

  /*!
   * \brief pause Emitted when the pause/resume button is checked/unchecked
   * \param pause Boolean representing the pause/resume state of the
   * pause/resume button
   */
  void pause(const bool pause);

  /*!
   * \brief switchFrame Emitted when the settings button is clicked
   */
  void switchFrame();

 private slots:
  /*!
   * \brief togglePauseIcon Toggles the pause button icon and text when the
   * paused button is checked
   */
  void togglePauseIcon(const bool toggle);

 private:
  class HeaderFramePrivate;
  pimpl<HeaderFramePrivate> m;
};

#endif // KRYVOS_GUI_HEADERFRAME_HPP_
