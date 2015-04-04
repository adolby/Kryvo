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

#include "utility/make_unique.h"
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QWidget>
#include <QtCore/QRectF>
#include <QtCore/QString>
#include <memory>

class SlideSwitch : public QAbstractButton {
  Q_OBJECT

 public:
  explicit SlideSwitch(QWidget* parent = nullptr);
  explicit SlideSwitch(const QString& text, QWidget* parent = nullptr);
  virtual ~SlideSwitch();

 public:
  void setBackgroundPixmap(const QString& backgroundPath);
  void setKnobPixmaps(const QString& knobEnabledPath,
                      const QString& knobDisabledPath);

 protected:
  virtual void paintEvent(QPaintEvent* /*event*/);
  virtual QSize sizeHint() const;
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* /*event*/);
  bool hitButton(const QPoint& pos) const;

 private slots:
  void setSwitchPosition(int position);
  void updateSwitchPosition(bool checked);

 private:
  QRectF buttonRect() const;
  QRectF knobRect() const;

 private:
  class SlideSwitchPrivate;
  std::unique_ptr<SlideSwitchPrivate> pimpl;
};

#endif // KRYVOS_GUI_SLIDESWITCH_HPP_
