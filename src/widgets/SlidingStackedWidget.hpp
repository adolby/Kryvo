#ifndef KRYVO_WIDGETS_SLIDINGSTACKEDWIDGET_HPP_
#define KRYVO_WIDGETS_SLIDINGSTACKEDWIDGET_HPP_

#include "utility/pimpl.h"
#include <QStackedWidget>
#include <QWidget>
#include <QEasingCurve>
#include <memory>

namespace Kryvo {

class SlidingStackedWidgetPrivate;

/*!
 * SlidingStackedWidget is a class that is derived from QStackedWidget
 * and allows smooth side shifting of widgets, in addition
 * to the original hard switching from one to another as offered by
 * QStackedWidget.
*/
class SlidingStackedWidget : public QStackedWidget {
  Q_OBJECT
  Q_DISABLE_COPY(SlidingStackedWidget)
  DECLARE_PRIVATE(SlidingStackedWidget)
  std::unique_ptr<SlidingStackedWidgetPrivate> const d_ptr;

 public:
  //! This enumeration is used to define the animation direction.
  enum Direction {
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop,
    Automatic
  };

  explicit SlidingStackedWidget(QWidget* parent = nullptr);

  ~SlidingStackedWidget() override;

 signals:
  void animationFinished();

 public slots:
  void setSpeed(int speed);
  void setAnimation(QEasingCurve::Type animationType);
  void setVerticalMode(bool vertical);
  void setWrap(bool wrap);

  void slideInNext();
  void slideInPrev();
  void slideInIndex(int index, Direction direction = Automatic);

 private slots:
  void animationDone();

 private:
  void slideInWidget(QWidget* widget, Direction direction);
  void stopAnimation();
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_SLIDINGSTACKEDWIDGET_HPP_
