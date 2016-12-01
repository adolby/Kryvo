#include "src/gui/SlidingStackedWidget.hpp"
#include "src/utility/pimpl_impl.h"
#include <QPushButton>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QByteArray>

class SlidingStackedWidget::SlidingStackedWidgetPrivate {
 public:
  /*!
   * \brief SlidingStackedWidgetPrivate Constructs the SlidingStackedWidgetFrame
   * private implementation.
   */
  SlidingStackedWidgetPrivate();

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
  : QStackedWidget{parent}
{
  m->animationGroup->setParent(this);

  connect(m->animationGroup, &QParallelAnimationGroup::finished,
          this, &SlidingStackedWidget::animationDone);
}

SlidingStackedWidget::~SlidingStackedWidget()
{}

void SlidingStackedWidget::setSpeed(const int speed)
{
  m->speed = speed;
}

void SlidingStackedWidget::setAnimation(QEasingCurve::Type animationType)
{
  m->animationType = animationType;
}

void SlidingStackedWidget::setVerticalMode(const bool vertical)
{
  m->vertical = vertical;
}

void SlidingStackedWidget::setWrap(const bool wrap)
{
  m->wrap = wrap;
}

void SlidingStackedWidget::slideInNext()
{
  stopAnimation();

  auto index = currentIndex();

  if (m->wrap || (index < count() - 1))
  {
    slideInIndex(index + 1);
  }
}

void SlidingStackedWidget::slideInPrev()
{
  stopAnimation();

  auto index = currentIndex();

  if (m->wrap || (index > 0))
  {
    slideInIndex(index - 1);
  }
}

void SlidingStackedWidget::slideInIndex(const int index,
                                        const Direction direction)
{
  auto updatedIndex = index;
  auto updatedDirection = direction;

  // Bound index and direction to stack
  if (index > count() - 1)
  {
    updatedDirection = m->vertical ? TopToBottom : RightToLeft;
    updatedIndex = index % count();
  }
  else if (index < 0)
  {
    updatedDirection = m->vertical ? BottomToTop : LeftToRight;
    updatedIndex = (index + count()) % count();
  }

  slideInWidget(widget(updatedIndex), updatedDirection);
}

void SlidingStackedWidget::animationDone()
{
  setCurrentIndex(m->nextIndex);

  // Reset the position of the out-shifted widget
  widget(m->currentIndex)->move(m->lastWidgetPos);

  emit animationFinished();
}

void SlidingStackedWidget::slideInWidget(QWidget* nextWidget,
                                         const Direction direction)
{
  auto currentIdx = currentIndex();
  auto nextIdx = indexOf(nextWidget);

  if (currentIdx != nextIdx)
  {
    auto directionHint = direction;

    if (directionHint == Automatic)
    {
      if (currentIdx < nextIdx)
      {
        directionHint = m->vertical ? TopToBottom : RightToLeft;
      }
      else
      {
        directionHint = m->vertical ? BottomToTop : LeftToRight;
      }
    }

    auto offsetX = frameRect().width();
    auto offsetY = frameRect().height();

    auto currentWidget = widget(currentIdx);
    auto nextWidget = widget(nextIdx);

    // Ensure that the next widget has correct geometry information when
    // sliding in for the first time
    nextWidget->setGeometry(0, 0, offsetX, offsetY);

    if (directionHint == TopToBottom)
    {
      offsetX = 0;
      offsetY = -offsetY;
    }
    else if (directionHint == BottomToTop)
    {
      offsetX = 0;
    }
    else if (directionHint == RightToLeft)
    {
      offsetX = -offsetX;
      offsetY = 0;
    }
    else if (directionHint == LeftToRight)
    {
      offsetY = 0;
    }

    auto currentWidgetPos = currentWidget->pos();
    // Store current widget position for re-positioning later
    m->lastWidgetPos = currentWidgetPos;

    auto nextWidgetPos = nextWidget->pos();
    nextWidget->move(nextWidgetPos.x() - offsetX,
                     nextWidgetPos.y() - offsetY);

    // Show next widget
    nextWidget->show();

    // Animate both the current and next widget to the side
    m->currentWidgetAnimation->setTargetObject(currentWidget);
    m->currentWidgetAnimation->setDuration(m->speed);
    m->currentWidgetAnimation->setEasingCurve(m->animationType);
    m->currentWidgetAnimation->setStartValue(currentWidgetPos);
    m->currentWidgetAnimation->setEndValue(QPoint{offsetX +
                                                  currentWidgetPos.x(),
                                                  offsetY +
                                                  currentWidgetPos.y()});

    m->nextWidgetAnimation->setTargetObject(nextWidget);
    m->nextWidgetAnimation->setDuration(m->speed);
    m->nextWidgetAnimation->setEasingCurve(m->animationType);
    m->nextWidgetAnimation->setStartValue(QPoint{-offsetX +
                                                 nextWidgetPos.x(),
                                                 offsetY +
                                                 nextWidgetPos.y()});
    m->nextWidgetAnimation->setEndValue(nextWidgetPos);

    m->nextIndex = nextIdx;
    m->currentIndex = currentIdx;
    m->animationGroup->start();
  }
}

void SlidingStackedWidget::stopAnimation()
{
  if (m->animationGroup->state() == QAbstractAnimation::Running)
  {
    m->animationGroup->stop();

    setCurrentIndex(m->nextIndex);

    // Reset the position of the shifted widgets
    widget(m->currentIndex)->move(m->lastWidgetPos);
    widget(m->nextIndex)->move(m->lastWidgetPos);
  }
}

SlidingStackedWidget::SlidingStackedWidgetPrivate::SlidingStackedWidgetPrivate()
  : speed{500}, animationType{QEasingCurve::InOutSine}, vertical{false},
    currentIndex{0}, nextIndex{0}, wrap{false}
{
  currentWidgetAnimation = new QPropertyAnimation{};
  currentWidgetAnimation->setPropertyName(QByteArrayLiteral("pos"));

  nextWidgetAnimation = new QPropertyAnimation{};
  nextWidgetAnimation->setPropertyName(QByteArrayLiteral("pos"));

  animationGroup = new QParallelAnimationGroup{};

  animationGroup->addAnimation(currentWidgetAnimation);
  animationGroup->addAnimation(nextWidgetAnimation);
}
