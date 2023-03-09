#include "SlidingStackedWidget.hpp"
#include <QPushButton>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QByteArray>

namespace Kryvo {

class SlidingStackedWidgetPrivate {
  Q_DISABLE_COPY(SlidingStackedWidgetPrivate)

 public:
  /*!
   * \brief SlidingStackedWidgetPrivate Constructs the SlidingStackedWidgetFrame
   * private implementation.
   */
  SlidingStackedWidgetPrivate();

  int speed{500};
  QEasingCurve::Type animationType{QEasingCurve::InOutSine};
  bool vertical{false};
  int currentIndex{0};
  int nextIndex{0};
  bool wrap{false};
  QPoint lastWidgetPos;

  QPropertyAnimation* currentWidgetAnimation{nullptr};
  QPropertyAnimation* nextWidgetAnimation{nullptr};
  QParallelAnimationGroup* animationGroup{nullptr};
};

SlidingStackedWidgetPrivate::SlidingStackedWidgetPrivate() {
  currentWidgetAnimation = new QPropertyAnimation();
  currentWidgetAnimation->setPropertyName(QByteArrayLiteral("pos"));

  nextWidgetAnimation = new QPropertyAnimation();
  nextWidgetAnimation->setPropertyName(QByteArrayLiteral("pos"));

  animationGroup = new QParallelAnimationGroup();

  animationGroup->addAnimation(currentWidgetAnimation);
  animationGroup->addAnimation(nextWidgetAnimation);
}

SlidingStackedWidget::SlidingStackedWidget(QWidget* parent)
  : QStackedWidget(parent),
    d_ptr(std::make_unique<SlidingStackedWidgetPrivate>()) {
  Q_D(SlidingStackedWidget);

  d->animationGroup->setParent(this);

  connect(d->animationGroup, &QParallelAnimationGroup::finished,
          this, &SlidingStackedWidget::animationDone);
}

SlidingStackedWidget::~SlidingStackedWidget() = default;

void SlidingStackedWidget::setSpeed(const int speed) {
  Q_D(SlidingStackedWidget);

  d->speed = speed;
}

void SlidingStackedWidget::setAnimation(QEasingCurve::Type animationType) {
  Q_D(SlidingStackedWidget);

  d->animationType = animationType;
}

void SlidingStackedWidget::setVerticalMode(const bool vertical) {
  Q_D(SlidingStackedWidget);

  d->vertical = vertical;
}

void SlidingStackedWidget::setWrap(const bool wrap) {
  Q_D(SlidingStackedWidget);

  d->wrap = wrap;
}

void SlidingStackedWidget::slideInNext() {
  Q_D(SlidingStackedWidget);

  stopAnimation();

  const int index = currentIndex();

  if (d->wrap || index < (count() - 1)) {
    slideInIndex(index + 1);
  }
}

void SlidingStackedWidget::slideInPrev() {
  Q_D(SlidingStackedWidget);

  stopAnimation();

  const int index = currentIndex();

  if (d->wrap || index > 0) {
    slideInIndex(index - 1);
  }
}

void SlidingStackedWidget::slideInIndex(const int index,
                                        const Direction direction) {
  Q_D(SlidingStackedWidget);

  int updatedIndex = index;
  Direction updatedDirection = direction;

  // Bound index and direction to stack
  if (index > count() - 1) {
    updatedDirection = d->vertical ? TopToBottom : RightToLeft;
    updatedIndex = index % count();
  }
  else if (index < 0) {
    updatedDirection = d->vertical ? BottomToTop : LeftToRight;
    updatedIndex = (index + count()) % count();
  }

  slideInWidget(widget(updatedIndex), updatedDirection);
}

void SlidingStackedWidget::animationDone() {
  Q_D(SlidingStackedWidget);

  setCurrentIndex(d->nextIndex);

  // Reset the position of the out-shifted widget
  widget(d->currentIndex)->move(d->lastWidgetPos);

  emit animationFinished();
}

void SlidingStackedWidget::slideInWidget(QWidget* nextWidget,
                                         const Direction direction) {
  Q_D(SlidingStackedWidget);

  const int currentIdx = currentIndex();
  const int nextIdx = indexOf(nextWidget);

  if (currentIdx != nextIdx) {
    Direction directionHint = direction;

    if (directionHint == Automatic) {
      if (currentIdx < nextIdx) {
        directionHint = d->vertical ? TopToBottom : RightToLeft;
      }
      else {
        directionHint = d->vertical ? BottomToTop : LeftToRight;
      }
    }

    int offsetX = frameRect().width();
    int offsetY = frameRect().height();

    QWidget* currentWidget = widget(currentIdx);
    QWidget* nextWidget = widget(nextIdx);

    // Ensure that the next widget has correct geometry information when
    // sliding in for the first time
    nextWidget->setGeometry(0, 0, offsetX, offsetY);

    if (directionHint == TopToBottom) {
      offsetX = 0;
      offsetY = -offsetY;
    }
    else if (directionHint == BottomToTop) {
      offsetX = 0;
    }
    else if (directionHint == RightToLeft) {
      offsetX = -offsetX;
      offsetY = 0;
    }
    else if (directionHint == LeftToRight) {
      offsetY = 0;
    }

    const QPoint currentWidgetPos = currentWidget->pos();

    // Store current widget position for re-positioning later
    d->lastWidgetPos = currentWidgetPos;

    const QPoint nextWidgetPos = nextWidget->pos();
    nextWidget->move(nextWidgetPos.x() - offsetX,
                     nextWidgetPos.y() - offsetY);

    // Show next widget
    nextWidget->show();

    // Animate both the current and next widget to the side
    d->currentWidgetAnimation->setTargetObject(currentWidget);
    d->currentWidgetAnimation->setDuration(d->speed);
    d->currentWidgetAnimation->setEasingCurve(d->animationType);
    d->currentWidgetAnimation->setStartValue(currentWidgetPos);
    d->currentWidgetAnimation->setEndValue(QPoint(offsetX +
                                                  currentWidgetPos.x(),
                                                  offsetY +
                                                  currentWidgetPos.y()));

    d->nextWidgetAnimation->setTargetObject(nextWidget);
    d->nextWidgetAnimation->setDuration(d->speed);
    d->nextWidgetAnimation->setEasingCurve(d->animationType);
    d->nextWidgetAnimation->setStartValue(QPoint(-offsetX +
                                                 nextWidgetPos.x(),
                                                 offsetY +
                                                 nextWidgetPos.y()));
    d->nextWidgetAnimation->setEndValue(nextWidgetPos);

    d->nextIndex = nextIdx;
    d->currentIndex = currentIdx;
    d->animationGroup->start();
  }
}

void SlidingStackedWidget::stopAnimation() {
  Q_D(SlidingStackedWidget);

  if (d->animationGroup->state() == QAbstractAnimation::Running) {
    d->animationGroup->stop();

    setCurrentIndex(d->nextIndex);

    // Reset the position of the shifted widgets
    widget(d->currentIndex)->move(d->lastWidgetPos);
    widget(d->nextIndex)->move(d->lastWidgetPos);
  }
}

} // namespace Kryvo
