#ifndef KRYVO_WIDGETS_DESKTOPMAINWINDOW_HPP_
#define KRYVO_WIDGETS_DESKTOPMAINWINDOW_HPP_

#include "MainWindow.hpp"
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
  Q_DISABLE_COPY(DesktopMainWindow)

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
  void settingsImported() override;

  /*!
  * \brief closeEvent Executed when the main window is closed
  * \param event Close event
  */
  void closeEvent(QCloseEvent* event) override;

  /*!
  * \brief dragEnterEvent Executed when a drag enter event occurs on the main
  * window
  * \param event Drag enter event
  */
  void dragEnterEvent(QDragEnterEvent* event) override;

  /*!
  * \brief dropEvent Executed when a drop event occurs on the main window
  * \param event Drop event
  */
  void dropEvent(QDropEvent* event) override;

  /*!
   * \brief sizeHint Returns the preferred size of the main window
   * \return Preferred size
   */
  QSize sizeHint() const override;

  /*!
   * \brief minimumSizeHint Returns the minimum size of the main window
   * \return Maximum size
   */
  QSize minimumSizeHint() const override;
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_DESKTOPMAINWINDOW_HPP_
