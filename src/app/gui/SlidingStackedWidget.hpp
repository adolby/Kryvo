#ifndef KRYVO_GUI_SLIDINGSTACKEDWIDGET_HPP_
#define KRYVO_GUI_SLIDINGSTACKEDWIDGET_HPP_

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

  ~SlidingStackedWidget();

 signals:
  void animationFinished();

 public slots:
  void setSpeed(const int speed);
  void setAnimation(const QEasingCurve::Type animationType);
  void setVerticalMode(const bool vertical);
  void setWrap(const bool wrap);

  void slideInNext();
  void slideInPrev();
  void slideInIndex(const int index, const Direction direction = Automatic);

 private slots:
  void animationDone();

 private:
  void slideInWidget(QWidget* widget, const Direction direction);
  void stopAnimation();
};

}

#endif // KRYVO_GUI_SLIDINGSTACKEDWIDGET_HPP_
