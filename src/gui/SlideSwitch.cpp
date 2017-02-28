#include "src/gui/SlideSwitch.hpp"
#include "src/utility/pimpl_impl.h"
#include <QMouseEvent>
#include <QPainter>
#include <QTimeLine>

//#include <QDebug>

class Kryvos::SlideSwitch::SlideSwitchPrivate {
 public:
  SlideSwitchPrivate();

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

Kryvos::SlideSwitch::SlideSwitch(QWidget* parent)
  : QAbstractButton(parent) {
  setCheckable(true);
  setChecked(false);

  m->timeLine = new QTimeLine{150, this};
  m->timeLine->setCurveShape(QTimeLine::EaseInCurve);

  connect(m->timeLine, &QTimeLine::frameChanged,
          this, &SlideSwitch::setSwitchPosition);

  connect(this, &SlideSwitch::toggled,
          this, &SlideSwitch::updateSwitchPosition);

  setAttribute(Qt::WA_Hover);
}

Kryvos::SlideSwitch::~SlideSwitch() {
}

void Kryvos::SlideSwitch::setBackgroundPixmap(const QString& backgroundPath) {
  m->background = QPixmap(backgroundPath);
}

void Kryvos::SlideSwitch::setKnobPixmaps(const QString& knobEnabledPath,
                                 const QString& knobDisabledPath) {
  m->knobEnabled = QPixmap(knobEnabledPath);
  m->knobDisabled = QPixmap(knobDisabledPath);
}

void Kryvos::SlideSwitch::paintEvent(QPaintEvent* /*event*/) {
  QPainter painter(this);

  //painter.drawPixmap(buttonRect().toRect(), m->background);

  if (isChecked()) {
    //painter.drawPixmap(knobRect().toRect(), m->knobEnabled);
  }
  else {
    //painter.drawPixmap(knobRect().toRect(), m->knobDisabled);
  }
}

QSize Kryvos::SlideSwitch::sizeHint() const {
  if (!m->background.isNull()) {
    return m->background.size();
  }
  else {
    return QSize(80, 38);
  }
}

void Kryvos::SlideSwitch::mouseMoveEvent(QMouseEvent* event) {
  if(m->dragInProgress) {
    m->dragDistanceX = event->x() - m->dragStartPosition.x();

    if (isChecked()) {
      m->position = 100 * (buttonRect().width() - knobRect().width() +
                           m->dragDistanceX) / (buttonRect().width() -
                                                knobRect().width());
    }
    else {
      m->position = 100 * m->dragDistanceX /
                        (buttonRect().width() - knobRect().width());
    }

    if (m->position >= 100) {
      m->position = 100;
      setChecked(true);
    }

    if (m->position <= 0) {
      m->position = 0;
      setChecked(false);
    }

    update();
  }
}

void Kryvos::SlideSwitch::mousePressEvent(QMouseEvent* event) {
  if (Qt::LeftButton == event->button() && knobRect().contains(event->pos())) {
    m->dragInProgress = true;
    m->dragStartPosition = event->pos();
  }
}

void Kryvos::SlideSwitch::mouseReleaseEvent(QMouseEvent* /*event*/) {
  if (m->dragDistanceX != 0) {
    if (m->position < 100) {
      if (isChecked()) {
        m->timeLine->setFrameRange(100 - m->position, 100);
      }
      else {
        m->timeLine->setFrameRange(m->position, 100);
      }
    }
    else {
      m->timeLine->setFrameRange(0, 100);
    }

    if (0 == m->position || 100 == m->position) {
      m->timeLine->start();
    }
  }

  m->dragDistanceX = 0;
  m->dragInProgress = false;
}

bool Kryvos::SlideSwitch::hitButton(const QPoint& pos) const {
  return buttonRect().contains(pos);
}

void Kryvos::SlideSwitch::setSwitchPosition(const int position) {
  m->position = isChecked() ? 100 - position : position;

  update();

  if (100 == m->position) {
    setChecked(true);
  }
  else if (0 == m->position) {
    setChecked(false);
  }
}

void Kryvos::SlideSwitch::updateSwitchPosition(const bool checked) {
  if (checked) {
    m->position = 100;
  }
  else {
    m->position = 0;
  }
}

QRectF Kryvos::SlideSwitch::buttonRect() const {
  QSizeF buttonSize = m->background.size();
  buttonSize.scale(size(), Qt::KeepAspectRatio);

  return QRectF(QPointF(0, 0), buttonSize);
}

QRectF Kryvos::SlideSwitch::knobRect() const {
  QRectF button = buttonRect();
  QSizeF knobSize = m->knobEnabled.size();
  knobSize.scale(button.size(), Qt::KeepAspectRatio);
  QRectF knobRect(button.topLeft(), knobSize);

  // move the rect to the current position
  qreal pos = button.left() + (button.width() - knobSize.width()) *
              static_cast<qreal>(m->position) / 100.0;

  pos = qMax(button.left(), qMin(pos, button.right() - knobSize.width()));

  knobRect.moveLeft(pos);

  return knobRect;
}

Kryvos::SlideSwitch::SlideSwitchPrivate::SlideSwitchPrivate()
  : timeLine{nullptr}, dragDistanceX{0}, dragInProgress{false}, position{0} {
}
