/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "SlideSwitch.hpp"
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtCore/QTimeLine>

#include <QDebug>

class SlideSwitch::SlideSwitchPrivate {
 public:
  explicit SlideSwitchPrivate();

  QPixmap background;
  QPixmap knobDisabled;
  QPixmap knobEnabled;

  QTimeLine* timeLine;

  // Point of the started drag
  QPoint dragStartPosition;

  // Actual drag distance from dragStartPosition
  int dragDistanceX;

  // Drag is still in progress (true)
  bool dragInProgress;

  // Current position of knob
  int position;
};

SlideSwitch::SlideSwitch(QWidget* parent) :
  QAbstractButton(parent), pimpl{make_unique<SlideSwitchPrivate>()}
{ 
  setCheckable(true);
  setChecked(false);

  pimpl->timeLine = new QTimeLine{150, this};
  pimpl->timeLine->setCurveShape(QTimeLine::EaseInCurve);

  connect(pimpl->timeLine, &QTimeLine::frameChanged,
          this, &SlideSwitch::setSwitchPosition);

  connect(this, &SlideSwitch::toggled,
          this, &SlideSwitch::updateSwitchPosition);

  setAttribute(Qt::WA_Hover);
}

SlideSwitch::~SlideSwitch()
{}

void SlideSwitch::setBackgroundPixmap(const QString& backgroundPath)
{
  Q_ASSERT(pimpl);

  pimpl->background = QPixmap(backgroundPath);
}

void SlideSwitch::setKnobPixmaps(const QString& knobEnabledPath,
                                 const QString& knobDisabledPath)
{
  Q_ASSERT(pimpl);

  pimpl->knobEnabled = QPixmap(knobEnabledPath);
  pimpl->knobDisabled = QPixmap(knobDisabledPath);
}

void SlideSwitch::paintEvent(QPaintEvent* /*event*/)
{
  Q_ASSERT(pimpl);

  QPainter painter(this);

  painter.drawPixmap(buttonRect().toRect(), pimpl->background);

  if (isChecked())
  {
    painter.drawPixmap(knobRect().toRect(), pimpl->knobEnabled);
  }
  else
  {
    painter.drawPixmap(knobRect().toRect(), pimpl->knobDisabled);
  }
}

QRectF SlideSwitch::buttonRect() const
{
  Q_ASSERT(pimpl);

  QSizeF buttonSize = pimpl->background.size();
  buttonSize.scale(size(), Qt::KeepAspectRatio);

  return QRectF(QPointF(0, 0), buttonSize);
}

/*!
 * \brief SlideSwitch::knobRect Calculates the possible SlideSwitch knob in the widget.
 * \return
 */
QRectF SlideSwitch::knobRect() const
{
  Q_ASSERT(pimpl);

  QRectF button = buttonRect();
  QSizeF knobSize = pimpl->knobEnabled.size();
  knobSize.scale(button.size(), Qt::KeepAspectRatio);
  QRectF knobRect(button.topLeft(), knobSize);

  // move the rect to the current position
  qreal pos = button.left() + (button.width() - knobSize.width()) *
              static_cast<qreal>(pimpl->position) / 100.0;

  pos = qMax(button.left(), qMin(pos, button.right() - knobSize.width()));

  knobRect.moveLeft(pos);

  return knobRect;
}

/*!
 * \brief SlideSwitch::mouseMoveEvent The knob will be dragged to the moved position.
 * \param event
 */
void SlideSwitch::mouseMoveEvent(QMouseEvent* event)
{
  Q_ASSERT(pimpl);

  if(pimpl->dragInProgress)
  {
    pimpl->dragDistanceX = event->x() - pimpl->dragStartPosition.x();

    if (isChecked())
    {
      pimpl->position = 100 * (buttonRect().width() - knobRect().width() +
                               pimpl->dragDistanceX) / (buttonRect().width() -
                                                        knobRect().width());
    }
    else
    {
      pimpl->position = 100 * pimpl->dragDistanceX /
                        (buttonRect().width() - knobRect().width());
    }

    qDebug() << pimpl->position;

    if (pimpl->position >= 100)
    {
      pimpl->position = 100;
      setChecked(true);
    }

    if (pimpl->position <= 0)
    {
      pimpl->position = 0;
      setChecked(false);
    }

    update();
  }
}

/*!
  \overload
  \internal
  Overloaded function \a mousePressEventEvent().
*/
void SlideSwitch::mousePressEvent(QMouseEvent* event)
{
  Q_ASSERT(pimpl);

  if (Qt::LeftButton == event->button() && knobRect().contains(event->pos()))
  {
    pimpl->dragInProgress = true;
    pimpl->dragStartPosition = event->pos();
  }
}

/*!
  \overload
  \internal
  Overloaded function \a mouseReleaseEvent().
*/
void SlideSwitch::mouseReleaseEvent(QMouseEvent* /*event*/)
{
  Q_ASSERT(pimpl);

  if (pimpl->dragDistanceX != 0)
  {
    if (pimpl->position < 100)
    {
      if (isChecked())
      {
        pimpl->timeLine->setFrameRange(100 - pimpl->position, 100);
      }
      else
      {
        pimpl->timeLine->setFrameRange(pimpl->position, 100);
      }
    }
    else
    {
      pimpl->timeLine->setFrameRange(0, 100);
    }

    if (0 == pimpl->position || 100 == pimpl->position)
    {
      pimpl->timeLine->start();
    }
  }

  pimpl->dragDistanceX = 0;
  pimpl->dragInProgress = false;
}

/*!
  \overload
  \internal
  Check if the widget has been clicked. Overloaded to define own hit area.
*/
bool SlideSwitch::hitButton(const QPoint& pos) const
{
  return buttonRect().contains(pos);
}

/*!
  \internal
  Animation to change the state of the widget at the end of the
  set position or the start position. \n
  If one of the two possible states is reached the signal is sent.
*/
void SlideSwitch::setSwitchPosition(int position)
{
  Q_ASSERT(pimpl);

  pimpl->position = isChecked() ? 100 - position : position;

  update();

  if (100 == pimpl->position)
  {
    setChecked(true);
  }
  else if (0 == pimpl->position)
  {
    setChecked(false);
  }
}

/*!
    \internal
    Used to make sure the slider position is correct when the developer
    uses setChecked()
*/
void SlideSwitch::updateSwitchPosition(bool checked)
{
  Q_ASSERT(pimpl);

  if (checked)
  {
    pimpl->position = 100;
  }
  else
  {
    pimpl->position = 0;
  }
}

/*!
    \overload
    Return size hint provided by the SVG graphics.
    Can be changed during runtime.
*/
QSize SlideSwitch::sizeHint() const
{
  Q_ASSERT(pimpl);

  if (!pimpl->background.isNull())
  {
    return pimpl->background.size();
  }
  else
  {
    return QSize(80, 38);
  }
}

SlideSwitch::SlideSwitchPrivate::SlideSwitchPrivate() :
  timeLine{nullptr}, dragDistanceX{0}, dragInProgress{false}, position{0}
{}
