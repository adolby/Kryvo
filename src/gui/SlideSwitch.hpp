#ifndef KRYVOS_GUI_SLIDESWITCH_HPP_
#define KRYVOS_GUI_SLIDESWITCH_HPP_

#include "src/utility/pimpl.h"
#include <QAbstractButton>
#include <QWidget>
#include <QRectF>
#include <QString>

class SlideSwitch : public QAbstractButton {
  Q_OBJECT

 public:
  explicit SlideSwitch(QWidget* parent = nullptr);
  //explicit SlideSwitch(const QString& text, QWidget* parent = nullptr);
  virtual ~SlideSwitch();

  void setBackgroundPixmap(const QString& backgroundPath);
  void setKnobPixmaps(const QString& knobEnabledPath,
                      const QString& knobDisabledPath);

 protected:
  virtual void paintEvent(QPaintEvent* /*event*/);

  /*!
   * \overload
   * Return size hint provided by the SVG graphics.
   * Can be changed during runtime.
   */
  virtual QSize sizeHint() const;

  /*!
   * \brief SlideSwitch::mouseMoveEvent The knob will be dragged to the moved
   * position.
   * \param event
   */
  virtual void mouseMoveEvent(QMouseEvent* event);

  /*!
   * \overload
   * \internal
   * Overloaded function \a mousePressEventEvent().
   */
  virtual void mousePressEvent(QMouseEvent* event);

  /*!
   * \overload
   * \internal
   * Overloaded function \a mouseReleaseEvent().
   */
  virtual void mouseReleaseEvent(QMouseEvent* /*event*/);

  /*!
   * \overload
   * \internal
   * Check if the widget has been clicked. Overloaded to define own hit area.
   */
  bool hitButton(const QPoint& pos) const;

 private slots:
  /*!
   * \internal
   * Animation to change the state of the widget at the end of the
   * set position or the start position. \n
   * If one of the two possible states is reached the signal is sent.
   */
  void setSwitchPosition(const int position);

  /*!
   * \internal
   * Used to make sure the slider position is correct when the developer
   * uses setChecked()
   */
  void updateSwitchPosition(const bool checked);

 private:
  QRectF buttonRect() const;

  /*!
   * \brief SlideSwitch::knobRect Calculates the possible SlideSwitch knob in the
   * widget.
   * \return
   */
  QRectF knobRect() const;

 private:
  class SlideSwitchPrivate;
  pimpl<SlideSwitchPrivate> m;
};

#endif // KRYVOS_GUI_SLIDESWITCH_HPP_
