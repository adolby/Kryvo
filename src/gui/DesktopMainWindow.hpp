#ifndef KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_
#define KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_

#include "gui/MainWindow.hpp"
#include "settings/Settings.hpp"
#include <QtGui/QCloseEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QSize>

class DesktopMainWindow : public MainWindow
{
 public:
  DesktopMainWindow(Settings* settings, QWidget* parent = nullptr);

 protected:
  /*!
  * \brief closeEvent Executed when the main window is closed.
  * \param event Close event.
  */
  virtual void closeEvent(QCloseEvent* event);

  /*!
  * \brief dragEnterEvent Executed when a drag enter event occurs on the main
  * window.
  * \param event Drag enter event.
  */
  virtual void dragEnterEvent(QDragEnterEvent* event);

  /*!
  * \brief dropEvent Executed when a drop event occurs on the main window.
  * \param event Drop event.
  */
  virtual void dropEvent(QDropEvent* event);

  /*!
   * \brief sizeHint Returns the preferred size of the main window.
   * \return Preferred size.
   */
  virtual QSize sizeHint() const;

  /*!
   * \brief minimumSizeHint Returns the minimum size of the main window.
   * \return Maximum size.
   */
  virtual QSize minimumSizeHint() const;

 private:
  Settings* settings;
};

#endif // KRYVOS_GUI_DESKTOPMAINWINDOW_HPP_
