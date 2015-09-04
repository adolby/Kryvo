/**
 * Kryvos - Encrypts and decrypts files.
 * Copyright (C) 2014, 2015 Andrew Dolby
 *
 * This file is part of Kryvos.
 *
 * Kryvos is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kryvos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kryvos.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#ifndef KRYVOS_GUI_SLIDESWITCH_HPP_
#define KRYVOS_GUI_SLIDESWITCH_HPP_

#include "utility/pimpl.h"
#include "utility/make_unique.h"
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
