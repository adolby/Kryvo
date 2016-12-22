#ifndef KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_
#define KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_

#include "src/gui/MainWindow.hpp"
#include "src/settings/Settings.hpp"
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QSize>

namespace Kryvos {

inline namespace UI {

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
  explicit DesktopMainWindow(Settings* settings = nullptr,
                             QWidget* parent = nullptr);

  /*!
   * \brief ~DesktopMainWindow Destroys the application's main window for
   * desktop displays
   */
  virtual ~DesktopMainWindow();

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

}

#endif // KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_
