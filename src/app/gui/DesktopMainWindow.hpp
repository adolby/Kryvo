#ifndef KRYVO_GUI_DESKTOPMAINWINDOW_HPP_
#define KRYVO_GUI_DESKTOPMAINWINDOW_HPP_

#include "gui/MainWindow.hpp"
#include "settings/Settings.hpp"
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QSize>

namespace Kryvo {

/*!
 * \brief The DesktopMainWindow class is used as the main window for the
 * application when a desktop display is the primary display for the operating
 * system.
 */
class DesktopMainWindow : public MainWindow {
 public:
  /*!
   * \brief DesktopMainWindow Constructs the application's main window for
   * desktop displays
   * \param settings Settings object
   * \param parent Parent widget
   */
  explicit DesktopMainWindow(Settings* s = nullptr,
                             QWidget* parent = nullptr);

 protected:
  /*!
  * \brief closeEvent Executed when the main window is closed
  * \param event Close event
  */
  virtual void closeEvent(QCloseEvent* event);

  /*!
  * \brief dragEnterEvent Executed when a drag enter event occurs on the main
  * window
  * \param event Drag enter event
  */
  virtual void dragEnterEvent(QDragEnterEvent* event);

  /*!
  * \brief dropEvent Executed when a drop event occurs on the main window
  * \param event Drop event
  */
  virtual void dropEvent(QDropEvent* event);

  /*!
   * \brief sizeHint Returns the preferred size of the main window
   * \return Preferred size
   */
  virtual QSize sizeHint() const;

  /*!
   * \brief minimumSizeHint Returns the minimum size of the main window
   * \return Maximum size
   */
  virtual QSize minimumSizeHint() const;

 private:
  Settings* settings;
};

}

#endif // KRYVO_GUI_DESKTOPMAINWINDOW_HPP_
