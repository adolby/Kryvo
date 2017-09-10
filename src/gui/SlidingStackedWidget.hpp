#ifndef KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP_
#define KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP_

#include "src/utility/pimpl.h"
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>
#include <QtCore/QEasingCurve>

namespace Kryvos {

inline namespace UI {

/*!
 * SlidingStackedWidget is a class that is derived from QStackedWidget
 * and allows smooth side shifting of widgets, in addition
 * to the original hard switching from one to another as offered by
 * QStackedWidget.
*/
class SlidingStackedWidget : public QStackedWidget {
  Q_OBJECT

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
  virtual ~SlidingStackedWidget();

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

  class SlidingStackedWidgetPrivate;
  Kryvos::pimpl<SlidingStackedWidgetPrivate> m;
};

}

}

#endif // KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP_
