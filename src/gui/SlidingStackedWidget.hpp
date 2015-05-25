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

#ifndef KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP_
#define KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP_

#include "utility/pimpl.h"
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>
#include <QtCore/QEasingCurve>

/*!
 * SlidingStackedWidget is a class that is derived from QStackedWidget
 * and allows smooth side shifting of widgets, in addition
 * to the original hard switching from one to another as offered by
 * QStackedWidget.
*/
class SlidingStackedWidget : public QStackedWidget {
  Q_OBJECT

 public:
  //! This enumeration is used to define the animation direction.
  enum Direction
  {
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop,
    Automatic
  };

  explicit SlidingStackedWidget(QWidget* parent = nullptr);
  virtual ~SlidingStackedWidget();

 signals:
  void animationFinished();

 public slots:
  void setSpeed(const int speed);
  void setAnimation(const QEasingCurve::Type animationType);
  void setVerticalMode(const bool vertical);
  void setWrap(const bool wrap);

  void slideInNext();
  void slideInPrev();
  void slideInIndex(const int index, const Direction direction = Automatic);

 private slots:
  void animationDone();

 private:
  void slideInWidget(QWidget* widget, const Direction direction);
  void stopAnimation();

  class SlidingStackedWidgetPrivate;
  pimpl<SlidingStackedWidgetPrivate> m;
};

#endif // KRYVOS_GUI_SLIDINGSTACKEDWIDGET_HPP_
