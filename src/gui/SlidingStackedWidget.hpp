#ifndef KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP__
#define KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP__

#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>
#include <QtCore/QEasingCurve>
#include <memory>

/*!
 * SlidingStackedWidget is a class that is derived from QtStackedWidget
 * and allows smooth side shifting of widgets, in addition
 * to the original hard switching from one to another as offered by
 * QStackedWidget itself.
*/
class SlidingStackedWidget : public QStackedWidget {
  Q_OBJECT

 public:
  //! This enumeration is used to define the animation direction
  enum Direction
  {
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop,
    Automatic
  };

  explicit SlidingStackedWidget(QWidget* parent = nullptr);
  virtual ~SlidingStackedWidget();

 signals:
  void animationFinished();

 public slots:
  void setSpeed(int speed);
  void setAnimation(QEasingCurve::Type animationType);
  void setVerticalMode(bool vertical);
  void setWrap(bool wrap);

  void slideInNext();
  void slideInPrev();
  void slideInIndex(const int index, const Direction direction = Automatic);

 private slots:
  void animationDone();

 private:
  void slideInWidget(QWidget* widget, const Direction direction);
  void stopAnimation();

  class SlidingStackedWidgetPrivate;
  std::unique_ptr<SlidingStackedWidgetPrivate> pimpl;
};

#endif // KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP__
