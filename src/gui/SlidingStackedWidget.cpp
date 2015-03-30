#include "SlidingStackedWidget.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QPushButton>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPropertyAnimation>

class SlidingStackedWidget::SlidingStackedWidgetPrivate {
 public:
  /*!
   * \brief SlidingStackedWidgetPrivate Constructs the SlidingStackedWidgetFrame
   * private implementation.
   */
  explicit SlidingStackedWidgetPrivate();
  virtual ~SlidingStackedWidgetPrivate();

  int speed;
  QEasingCurve::Type animationType;
  bool vertical;
  int currentIndex;
  int nextIndex;
  bool wrap;
  QPoint lastWidgetPos;

  QPropertyAnimation* currentWidgetAnimation;
  QPropertyAnimation* nextWidgetAnimation;
  QParallelAnimationGroup* animationGroup;
};

SlidingStackedWidget::SlidingStackedWidget(QWidget* parent)
  : QStackedWidget{parent}, pimpl{make_unique<SlidingStackedWidgetPrivate>()}
{
  pimpl->animationGroup->setParent(this);

  QObject::connect(pimpl->animationGroup, &QParallelAnimationGroup::finished,
                   this, &SlidingStackedWidget::animationDone);
}

SlidingStackedWidget::~SlidingStackedWidget()
{}

void SlidingStackedWidget::setSpeed(int speed)
{
  pimpl->speed = speed;
}

void SlidingStackedWidget::setAnimation(QEasingCurve::Type animationType)
{
  pimpl->animationType = animationType;
}

void SlidingStackedWidget::setVerticalMode(bool vertical)
{
  pimpl->vertical = vertical;
}

void SlidingStackedWidget::setWrap(bool wrap)
{
  pimpl->wrap = wrap;
}

void SlidingStackedWidget::slideInNext()
{
  stopAnimation();

  auto index = currentIndex();

  if (pimpl->wrap || (index < count() - 1))
  {
    slideInIndex(index + 1);
  }
}

void SlidingStackedWidget::slideInPrev()
{
  stopAnimation();

  auto index = currentIndex();

  if (pimpl->wrap || (index > 0))
  {
    slideInIndex(index - 1);
  }
}

void SlidingStackedWidget::slideInIndex(int index, Direction direction)
{
  stopAnimation();

  // Bound index and direction to stack
  if (index > count() - 1)
  {
    direction = pimpl->vertical ? TopToBottom : RightToLeft;
    index = index % count();
  }
  else if (index < 0)
  {
    direction = pimpl->vertical ? BottomToTop : LeftToRight;
    index = (index + count()) % count();
  }

  slideInWidget(widget(index), direction);
}

void SlidingStackedWidget::animationDone()
{
  setCurrentIndex(pimpl->nextIndex);

  // Reset the position of the out-shifted widget
  widget(pimpl->currentIndex)->move(pimpl->lastWidgetPos);

  emit animationFinished();
}

void SlidingStackedWidget::slideInWidget(QWidget* nextWidget,
                                         Direction direction)
{
  auto currentIdx = currentIndex();
  auto nextIdx = indexOf(nextWidget);

  if (currentIdx != nextIdx)
  {
    Direction directionHint;

    if (currentIdx < nextIdx)
    {
      directionHint = pimpl->vertical ? TopToBottom : RightToLeft;
    }
    else
    {
      directionHint = pimpl->vertical ? BottomToTop : LeftToRight;
    }

    if (direction == Automatic)
    {
      direction = directionHint;
    }

    auto offsetX = frameRect().width();
    auto offsetY = frameRect().height();

    auto currentWidget = widget(currentIdx);
    auto nextWidget = widget(nextIdx);

    // Ensure that the next widget has correct geometry information when
    // sliding in for the first time
    nextWidget->setGeometry(0, 0, offsetX, offsetY);

    if (direction == TopToBottom)
    {
      offsetX = 0;
      offsetY = -offsetY;
    }
    else if (direction == TopToBottom)
    {
      offsetX = 0;
    }
    else if (direction == RightToLeft)
    {
      offsetX = -offsetX;
      offsetY = 0;
    }
    else if (direction == LeftToRight)
    {
      offsetY = 0;
    }

    auto currentWidgetPos = currentWidget->pos();
    // Store current widget position for re-positioning later
    pimpl->lastWidgetPos = currentWidgetPos;

    auto nextWidgetPos = nextWidget->pos();
    nextWidget->move(nextWidgetPos.x() - offsetX,
                     nextWidgetPos.y() - offsetY);

    // Show next widget
    nextWidget->show();

    // Animate both the current and next widget to the side
    pimpl->currentWidgetAnimation->setTargetObject(currentWidget);
    pimpl->currentWidgetAnimation->setDuration(pimpl->speed);
    pimpl->currentWidgetAnimation->setEasingCurve(pimpl->animationType);
    pimpl->currentWidgetAnimation->setStartValue(currentWidgetPos);
    pimpl->currentWidgetAnimation->setEndValue(QPoint{offsetX +
                                                      currentWidgetPos.x(),
                                                      offsetY +
                                                      currentWidgetPos.y()});

    pimpl->nextWidgetAnimation->setTargetObject(nextWidget);
    pimpl->nextWidgetAnimation->setDuration(pimpl->speed);
    pimpl->nextWidgetAnimation->setEasingCurve(pimpl->animationType);
    pimpl->nextWidgetAnimation->setStartValue(QPoint{-offsetX +
                                                     nextWidgetPos.x(),
                                                     offsetY +
                                                     nextWidgetPos.y()});
    pimpl->nextWidgetAnimation->setEndValue(nextWidgetPos);

    pimpl->nextIndex = nextIdx;
    pimpl->currentIndex = currentIdx;
    pimpl->animationGroup->start();
  }
}

void SlidingStackedWidget::stopAnimation()
{
  if (pimpl->animationGroup->state() == QAbstractAnimation::Running)
  {
    pimpl->animationGroup->stop();

    setCurrentIndex(pimpl->nextIndex);

    // Reset the position of the shifted widgets
    widget(pimpl->currentIndex)->move(pimpl->lastWidgetPos);
    widget(pimpl->nextIndex)->move(pimpl->lastWidgetPos);
  }
}

SlidingStackedWidget::SlidingStackedWidgetPrivate::SlidingStackedWidgetPrivate()
  : speed{350}, animationType{QEasingCurve::Linear}, vertical{false},
    currentIndex{0}, nextIndex{0}, wrap{false}
{
  currentWidgetAnimation = new QPropertyAnimation{};
  currentWidgetAnimation->setPropertyName("pos");

  nextWidgetAnimation = new QPropertyAnimation{};
  nextWidgetAnimation->setPropertyName("pos");

  animationGroup = new QParallelAnimationGroup{};

  animationGroup->addAnimation(currentWidgetAnimation);
  animationGroup->addAnimation(nextWidgetAnimation);
}

SlidingStackedWidget::SlidingStackedWidgetPrivate::
~SlidingStackedWidgetPrivate()
{}
