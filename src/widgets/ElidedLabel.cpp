#include "ElidedLabel.hpp"
#include <QFontMetrics>
#include <QPainter>
#include <QString>

class Kryvo::ElidedLabelPrivate {
  Q_DISABLE_COPY(ElidedLabelPrivate)

 public:
  ElidedLabelPrivate();

  QString text;
  Qt::Alignment align{Qt::AlignLeft | Qt::AlignVCenter};
  Qt::TextElideMode mode{Qt::ElideLeft};
};

Kryvo::ElidedLabelPrivate::ElidedLabelPrivate() = default;

Kryvo::ElidedLabel::ElidedLabel(QWidget* parent, Qt::WindowFlags f)
  : QFrame(parent, f), d_ptr(std::make_unique<ElidedLabelPrivate>()) {
}

Kryvo::ElidedLabel::ElidedLabel(const QString& text, QWidget* parent,
                                Qt::WindowFlags f)
  : QFrame(parent, f), d_ptr(std::make_unique<ElidedLabelPrivate>()) {
  Q_D(ElidedLabel);

  d->text = text;
}

Kryvo::ElidedLabel::~ElidedLabel() = default;

void Kryvo::ElidedLabel::updateLabel() {
  this->updateGeometry();
  this->update();
}

QString Kryvo::ElidedLabel::text() const {
  Q_D(const ElidedLabel);
  return d->text;
}

void Kryvo::ElidedLabel::setText(const QString& text) {
  Q_D(ElidedLabel);

  if (d->text != text) {
    d->text = text;
    updateLabel();
  }
}

Qt::Alignment Kryvo::ElidedLabel::alignment() const {
  Q_D(const ElidedLabel);

  return d->align;
}

void Kryvo::ElidedLabel::setAlignment(const Qt::Alignment alignment) {
  Q_D(ElidedLabel);

  if (d->align != alignment) {
    d->align = alignment;
    update(); // no geometry change, repaint is sufficient
  }
}

Qt::TextElideMode Kryvo::ElidedLabel::elideMode() const {
  Q_D(const ElidedLabel);

  return d->mode;
}

void Kryvo::ElidedLabel::setElideMode(const Qt::TextElideMode mode) {
  Q_D(ElidedLabel);

  if (d->mode != mode) {
    d->mode = mode;
    updateLabel();
  }
}

QSize Kryvo::ElidedLabel::sizeHint() const {
  Q_D(const ElidedLabel);

  const QFontMetrics& fm = fontMetrics();
  const QSize size(fm.width(d->text), fm.height());
  return size;
}

QSize Kryvo::ElidedLabel::minimumSizeHint() const {
  Q_D(const ElidedLabel);

  switch (d->mode) {
    case Qt::ElideNone:
      return sizeHint();
    default: {
      const QFontMetrics& fm = fontMetrics();
      const QSize size{fm.width(QStringLiteral("...")), fm.height()};
      return size;
    }
  }
}

void Kryvo::ElidedLabel::paintEvent(QPaintEvent* event) {
  Q_D(ElidedLabel);

  QFrame::paintEvent(event);

  QPainter p(this);
  const QRect& r = contentsRect();

  const QString elidedText =
    fontMetrics().elidedText(d->text, d->mode, r.width());

  p.drawText(r, d->align, elidedText);
}

void Kryvo::ElidedLabel::changeEvent(QEvent* event) {
  QFrame::changeEvent(event);

  switch (event->type()) {
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
      updateLabel();
    break;
    default:
      // nothing to do
    break;
  }
}
