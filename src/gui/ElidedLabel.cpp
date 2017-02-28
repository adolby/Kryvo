#include "src/gui/ElidedLabel.hpp"
#include "src/utility/pimpl_impl.h"
#include <QFontMetrics>
#include <QPainter>
#include <QString>

class Kryvos::ElidedLabel::ElidedLabelPrivate {
 public:
  /*!
   * \brief ElidedLabelPrivate Constructs the ElidedLabel private implementation
   */
  ElidedLabelPrivate();

  QString text;
  Qt::Alignment align;
  Qt::TextElideMode mode;
};

Kryvos::ElidedLabel::ElidedLabel(QWidget* parent, Qt::WindowFlags f)
  : QFrame{parent, f} {
}

Kryvos::ElidedLabel::ElidedLabel(const QString& text, QWidget* parent,
                                 Qt::WindowFlags f)
  : QFrame{parent, f} {
  m->text = text;
}

Kryvos::ElidedLabel::~ElidedLabel() {
}

void Kryvos::ElidedLabel::updateLabel() {
  this->updateGeometry();
  this->update();
}

QString Kryvos::ElidedLabel::text() const {
  return m->text;
}

void Kryvos::ElidedLabel::setText(const QString& text) {
  if (m->text != text) {
    m->text = text;
    updateLabel();
    //emit textChanged(text);
  }
}

Qt::Alignment Kryvos::ElidedLabel::alignment() const
{
  return m->align;
}

void Kryvos::ElidedLabel::setAlignment(Qt::Alignment alignment)
{
  if (m->align != alignment) {
    m->align = alignment;
    update(); // no geometry change, repaint is sufficient
  }
}

Qt::TextElideMode Kryvos::ElidedLabel::elideMode() const
{
  return m->mode;
}

void Kryvos::ElidedLabel::setElideMode(Qt::TextElideMode mode) {
  if (m->mode != mode) {
    m->mode = mode;
    updateLabel();
  }
}

QSize Kryvos::ElidedLabel::sizeHint() const {
  const QFontMetrics& fm = fontMetrics();
  QSize size{fm.width(m->text), fm.height()};
  return size;
}

QSize Kryvos::ElidedLabel::minimumSizeHint() const {
  switch (m->mode) {
    case Qt::ElideNone:
      return sizeHint();
    default: {
      const QFontMetrics& fm = fontMetrics();
      QSize size{fm.width("..."), fm.height()};
      return size;
    }
  }
}

void Kryvos::ElidedLabel::paintEvent(QPaintEvent* event) {
  QFrame::paintEvent(event);

  QPainter p{this};
  const QRect& r = contentsRect();

  const QString elidedText =
    fontMetrics().elidedText(m->text, m->mode, r.width());

  p.drawText(r, m->align, elidedText);
}

void Kryvos::ElidedLabel::changeEvent(QEvent* event) {
  QFrame::changeEvent(event);

  switch (event->type())
  {
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
      updateLabel();
    break;
    default:
      // nothing to do
    break;
  }
}

Kryvos::ElidedLabel::ElidedLabelPrivate::ElidedLabelPrivate()
  : align{Qt::AlignLeft | Qt::AlignVCenter}, mode{Qt::ElideLeft} {
}
