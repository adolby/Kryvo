#include "NotificationFrame.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>

/*!
 * \brief The MainWindow::MainWindowPrivate class
 */
class MainWindow::MainWindowPrivate
{
 public:
  MainWindowPrivate();

  QFrame* mainFrame;
  QVBoxLayout* mainLayout;
};

/*!
 * \brief MainWindow::MainWindowPrivate::MainWindowPrivate Constructs the
 * MainWindow private implementation. Sets nullptr values to widgets, layouts,
 * and settings.
 */
MainWindow::MainWindowPrivate::MainWindowPrivate() :
  mainFrame{nullptr}, mainLayout{nullptr} {}

NotificationFrame::NotificationFrame(QWidget *parent) :
  QFrame(parent)
{

}


