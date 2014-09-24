/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "gui/Delegate.hpp"
#include <QtWidgets/QApplication>
#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>

Delegate::Delegate(QObject* parent) :
  QStyledItemDelegate{parent}, focusBorderEnabled{false}
{}

void Delegate::setFocusBorderEnabled(bool enabled)
{
  focusBorderEnabled = enabled;
}

void Delegate::initStyleOption(QStyleOptionViewItem* option,
                               const QModelIndex& index) const
{
  QStyledItemDelegate::initStyleOption(option, index);

  if (!focusBorderEnabled && option->state & QStyle::State_HasFocus)
  {
    option->state = option->state & ~QStyle::State_HasFocus;
  }
}

void Delegate::paint(QPainter* painter,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) const
{
  const auto column = index.column();

  switch (column)
  {
    case 0:
    {
      QStyledItemDelegate::paint(painter, option, index);
      break;
    }
    case 1:
    {
      // Set up a QStyleOptionProgressBar to mimic the environment of a progress
      // bar.
      auto progressBarOption = QStyleOptionProgressBar{};
      progressBarOption.state = QStyle::State_Enabled;
      progressBarOption.direction = QApplication::layoutDirection();
      progressBarOption.rect = QRect{option.rect.x(),
                                     option.rect.y() + 1,
                                     option.rect.width(),
                                     option.rect.height() - 1};
      progressBarOption.fontMetrics = QApplication::fontMetrics();
      progressBarOption.minimum = 0;
      progressBarOption.maximum = 100;
      progressBarOption.textAlignment = Qt::AlignCenter;
      progressBarOption.textVisible = true;

      // Set the progress and text values of the style option.
      const auto progress = index.model()->data(index, Qt::DisplayRole).toInt();
      progressBarOption.progress = progress < 0 ? 0 : progress;
      progressBarOption.text = QString{"%1%"}.arg(progressBarOption.progress);

      // Draw the progress bar onto the view.
      QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                         &progressBarOption,
                                         painter);
      break;
    }
    case 2:
    {
      auto buttonOption = QStyleOptionButton{};
      buttonOption.state = QStyle::State_Enabled;
      buttonOption.direction = QApplication::layoutDirection();
      buttonOption.rect = QRect{option.rect.x(),
                                option.rect.y(),
                                option.rect.width(),
                                option.rect.height()};
      buttonOption.fontMetrics = QApplication::fontMetrics();
      buttonOption.features = QStyleOptionButton::Flat;
      const auto closeIcon = QIcon{":/images/closeFileIcon.png"};
      buttonOption.icon = closeIcon;
      const auto iconSize = QSize{static_cast<int>(option.rect.width() * 0.4),
                                  static_cast<int>(option.rect.height() * 0.4)};
      buttonOption.iconSize = iconSize;

      QApplication::style()->drawControl(QStyle::CE_PushButton,
                                         &buttonOption,
                                         painter);
      break;
    }
  }
}

bool Delegate::editorEvent(QEvent* event,
                           QAbstractItemModel* model,
                           const QStyleOptionViewItem& option,
                           const QModelIndex& index)
{
  if (2 == index.column())
  {
    if (QEvent::MouseButtonRelease == event->type() ||
        QEvent::MouseButtonDblClick == event->type())
    {
      auto mouseEvent = static_cast<QMouseEvent*>(event);

      if (Qt::LeftButton == mouseEvent->button() &&
          option.rect.contains(mouseEvent->pos()))
      {
        emit removeRow(index);
      }
    }
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}