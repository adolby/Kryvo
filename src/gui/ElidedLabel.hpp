#ifndef KRYVOS_GUI_ELIDEDLABEL_HPP_
#define KRYVOS_GUI_ELIDEDLABEL_HPP_

#include "src/utility/pimpl.h"
#include <QFrame>
#include <QPaintEvent>
#include <QWidget>
#include <QEvent>
#include <QString>

namespace Kryvos {

inline namespace UI {

class ElidedLabel : public QFrame {
  Q_OBJECT

 public:
  explicit ElidedLabel(QWidget* parent = nullptr,
                       Qt::WindowFlags f = Qt::WindowFlags());

  explicit ElidedLabel(const QString& text,
                       QWidget* parent = nullptr,
                       Qt::WindowFlags f = Qt::WindowFlags());

  virtual ~ElidedLabel();

  void updateLabel();
  QString text() const;
  void setText(const QString& text);
  Qt::Alignment alignment() const;
  void setAlignment(Qt::Alignment alignment);
  Qt::TextElideMode elideMode() const;
  void setElideMode(Qt::TextElideMode mode);

 protected:
  QSize sizeHint() const;
  QSize minimumSizeHint() const;
  void paintEvent(QPaintEvent* event);
  void changeEvent(QEvent* event);

 private:
  class ElidedLabelPrivate;
  pimpl<ElidedLabelPrivate> m;
};

}

}

#endif // KRYVOS_GUI_ELIDEDLABEL_HPP_
