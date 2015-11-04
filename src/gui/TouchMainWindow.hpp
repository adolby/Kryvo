#ifndef KRYVOS_GUI_TOUCHMAINWINDOW_HPP_
#define KRYVOS_GUI_TOUCHMAINWINDOW_HPP_

#include "gui/MainWindow.hpp"
#include "settings/Settings.hpp"

/*!
 * \brief The TouchMainWindow class is used as the main window for the
 * application when a touch-based display is the primary display for the
 * operating system.
 */
class TouchMainWindow : public MainWindow
{
 public:
  /*!
   * \brief TouchMainWindow Constructs the application's main window for
   * touch-based displays.
   * \param settings Settings object
   * \param parent Widget parent
   */
  explicit TouchMainWindow(Settings* settings = nullptr,
                           QWidget* parent = nullptr);

  /*!
   * \brief ~TouchMainWindow Destroys the application's main window for
   * touch-based displays.
   */
  ~TouchMainWindow();
};

#endif // KRYVOS_GUI_TOUCHMAINWINDOW_HPP_
