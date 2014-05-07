#ifndef KRYVOS_GUI_NOTIFICATIONFRAME_HPP_
#define KRYVOS_GUI_NOTIFICATIONFRAME_HPP_

#include <QFrame>

class NotificationFrame : public QFrame
{
  Q_OBJECT

 public:
  explicit NotificationFrame(QWidget* parent = nullptr);

 private:
  class NotificationFramePrivate;
  std::unique_ptr<NotificationFramePrivate> pimpl;
};

#endif // KRYVOS_GUI_NOTIFICATIONFRAME_HPP_
