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

#ifndef KRYVOS_GUI_FLUIDLAYOUT_HPP_
#define KRYVOS_GUI_FLUIDLAYOUT_HPP_

#include "gui/flowlayout.h"
#include <QWidget>

/*!
 * \brief The FluidLayout class expands the FlowLayout class, allowing it to
 * properly report a vertical height representing the height of its content
 * widgets to parent layouts.
 */
class FluidLayout : public FlowLayout
{
 public:
  /*!
   * \brief FluidLayout Constructs a fluid layout with a parent.
   * \param parent Widget parent
   * \param margin Integer representing margin
   * \param hSpacing Integer representing horizontal spacing
   * \param vSpacing Integer representing vertical spacing
   */
  explicit FluidLayout(QWidget* parent, int margin = -1,
                       int hSpacing = -1, int vSpacing = -1);

  /*!
   * \brief FluidLayout Constructs a fluid layout.
   * \param margin Integer representing margin
   * \param hSpacing Integer representing horizontal spacing
   * \param vSpacing Integer representing vertical spacing
   */
  explicit FluidLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);

  /*!
   * \brief heightForWidth Calculates the height for width accounting for line
   * count.
   * \return Width
   */
  int heightForWidth(int width) const;

  /*!
   * \brief minimumSize Calculates the minimum size accounting for the line
   * count.
   * \return Minimum size for the fluid layout
   */
  QSize minimumSize() const;

  /*!
   * \brief setGeometry Sets the geometry for this layout manager to the
   * parameter rectangle rect.
   * \param rect New geometry rectangle
   */
  void setGeometry(const QRect& rect);

 protected:
  /*!
   * \brief doLayout Calculates the layout geometry and updates the line count
   * accordingly, which allows the layout manager to return an accurate size
   * of its children.
   * \param rect New geometry rectangle
   */
  void doLayout(const QRect& rect);

  /*!
   * \brief checkLayout Returns the vertical height of the layout, calculated
   * from its children.
   * \param rect New geometry rectangle
   * \return Vertical layout height
   */
  int checkLayout(const QRect& rect) const;

 private:
  // Number of lines children currently occupy in this layout manager
  int lineCount;
};

#endif // KRYVOS_GUI_FLUIDLAYOUT_HPP_
