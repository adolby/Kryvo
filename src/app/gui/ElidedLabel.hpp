#ifndef KRYVO_GUI_ELIDEDLABEL_HPP_
#define KRYVO_GUI_ELIDEDLABEL_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <QPaintEvent>
#include <QWidget>
#include <QEvent>
#include <QString>
#include <memory>

namespace Kryvo {

class ElidedLabelPrivate;

class ElidedLabel : public QFrame {
  Q_OBJECT
  DECLARE_PRIVATE(ElidedLabel)
  std::unique_ptr<ElidedLabelPrivate> const d_ptr;

 public:
  explicit ElidedLabel(QWidget* parent = nullptr,
                       Qt::WindowFlags f = Qt::WindowFlags());

  explicit ElidedLabel(const QString& text,
                       QWidget* parent = nullptr,
                       Qt::WindowFlags f = Qt::WindowFlags());

  ~ElidedLabel() override;

  void updateLabel();
  QString text() const;
  void setText(const QString& text);
  Qt::Alignment alignment() const;
  void setAlignment(const Qt::Alignment alignment);
  Qt::TextElideMode elideMode() const;
  void setElideMode(const Qt::TextElideMode mode);

 protected:
  QSize sizeHint() const;
  QSize minimumSizeHint() const;
  void paintEvent(QPaintEvent* event);
  void changeEvent(QEvent* event);
};

} // namespace Kryvo

#endif // KRYVO_GUI_ELIDEDLABEL_HPP_
