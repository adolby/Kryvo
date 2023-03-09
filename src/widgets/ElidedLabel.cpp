#include "ElidedLabel.hpp"
#include <QFontMetrics>
#include <QPainter>
#include <QString>

namespace Kryvo {

class ElidedLabelPrivate {
  Q_DISABLE_COPY(ElidedLabelPrivate)

 public:
  ElidedLabelPrivate();

  QString text;
  Qt::Alignment align{Qt::AlignLeft | Qt::AlignVCenter};
  Qt::TextElideMode mode{Qt::ElideLeft};
};

ElidedLabelPrivate::ElidedLabelPrivate() = default;

ElidedLabel::ElidedLabel(QWidget* parent, Qt::WindowFlags f)
  : QFrame(parent, f), d_ptr(std::make_unique<ElidedLabelPrivate>()) {
}

ElidedLabel::ElidedLabel(const QString& text, QWidget* parent,
                         Qt::WindowFlags f)
  : QFrame(parent, f), d_ptr(std::make_unique<ElidedLabelPrivate>()) {
  Q_D(ElidedLabel);

  d->text = text;
}

ElidedLabel::~ElidedLabel() = default;

void ElidedLabel::updateLabel() {
  this->updateGeometry();
  this->update();
}

QString ElidedLabel::text() const {
  Q_D(const ElidedLabel);
  return d->text;
}

void ElidedLabel::setText(const QString& text) {
  Q_D(ElidedLabel);

  if (d->text != text) {
    d->text = text;
    updateLabel();
  }
}

Qt::Alignment ElidedLabel::alignment() const {
  Q_D(const ElidedLabel);

  return d->align;
}

void ElidedLabel::setAlignment(const Qt::Alignment alignment) {
  Q_D(ElidedLabel);

  if (d->align != alignment) {
    d->align = alignment;
    update(); // no geometry change, repaint is sufficient
  }
}

Qt::TextElideMode ElidedLabel::elideMode() const {
  Q_D(const ElidedLabel);

  return d->mode;
}

void ElidedLabel::setElideMode(const Qt::TextElideMode mode) {
  Q_D(ElidedLabel);

  if (d->mode != mode) {
    d->mode = mode;
    updateLabel();
  }
}

QSize ElidedLabel::sizeHint() const {
  Q_D(const ElidedLabel);

  const QFontMetrics fm = fontMetrics();

  const QRect textRect = fm.boundingRect(d->text);

  return textRect.size();
}

QSize ElidedLabel::minimumSizeHint() const {
  Q_D(const ElidedLabel);

  switch (d->mode) {
    case Qt::ElideNone:
      return sizeHint();
    default: {
      const QFontMetrics fm = fontMetrics();
      const QRect textRect = fm.boundingRect(QStringLiteral("..."));
      return textRect.size();
    }
  }
}

void ElidedLabel::paintEvent(QPaintEvent* event) {
  Q_D(ElidedLabel);

  QFrame::paintEvent(event);

  QPainter p(this);
  const QRect r = contentsRect();

  const QString elidedText =
    fontMetrics().elidedText(d->text, d->mode, r.width());

  p.drawText(r, d->align, elidedText);
}

void ElidedLabel::changeEvent(QEvent* event) {
  QFrame::changeEvent(event);

  switch (event->type()) {
    case QEvent::FontChange:
      Q_FALLTHROUGH();
    case QEvent::ApplicationFontChange:
      updateLabel();
    break;
    default:
      // nothing to do
    break;
  }
}

} // namespace Kryvo
