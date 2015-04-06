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

#ifndef KRYVOS_GUI_FILELISTDELEGATE_HPP_
#define KRYVOS_GUI_FILELISTDELEGATE_HPP_

#include <QtWidgets/QStyledItemDelegate>
#include <QtCore/QSize>

/*!
 * \brief The FileListDelegate class augments the view to display a progress bar and to
 * remove the focus border.
 */
class FileListDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  /*!
   * \brief FileListDelegate Constructs a delegate that augments a view.
   * \param parent Object parent of this delegate
   */
  explicit FileListDelegate(QObject* parent = nullptr);

 signals:
  /*!
   * \brief removeRow Emitted when the delegate needs to remove a row from its
   * model.
   * \param index Index of the row to remove
   */
  void removeRow(const QModelIndex& index);

 public:
  /*!
   * \brief setFocusBorderEnabled Enables/disables the focus dotted line border
   * that appears on click.
   * \param enabled Boolean representing whether to enable or disable the
   * focus border. True represents the enabled state.
   */
  void setFocusBorderEnabled(bool enabled);

 protected:
  /*!
   * \brief initStyleOption Initializes the style option for this delegate. Used
   * to remove the focus.
   * \param option Style view options
   * \param index Index of the current cell in the model
   */
  virtual void initStyleOption(QStyleOptionViewItem* option,
                               const QModelIndex& index) const;

  /*!
   * \brief paint Paints the progress bar and the close button.
   * \param painter Painter object
   * \param option Style view options object. Passed on to parent's paint
   * \param index Index of the current cell in the model to paint
   */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index) const;

  /*!
   * \brief editorEvent Event to handle mouse clicks on the remove button.
   * \param event Editing event
   * \param model Model
   * \param option Style option view item
   * \param index Index of the current cell in the model
   * \return
   */
  virtual bool editorEvent(QEvent* event,
                           QAbstractItemModel* model,
                           const QStyleOptionViewItem& option,
                           const QModelIndex& index);

 protected:
  bool focusBorderEnabled;
};

#endif // KRYVOS_GUI_FILELISTDELEGATE_HPP_
