#include "gui/FileListDelegate.hpp"
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>

#include <QDebug>

Kryvo::FileListDelegate::FileListDelegate(QObject* parent)
  : QStyledItemDelegate{parent}, focusBorderEnabled{false} {
}

void Kryvo::FileListDelegate::setFocusBorderEnabled(bool enabled) {
  focusBorderEnabled = enabled;
}

void Kryvo::FileListDelegate::initStyleOption(QStyleOptionViewItem* option,
                                              const QModelIndex& index) const {
  QStyledItemDelegate::initStyleOption(option, index);

  if (!focusBorderEnabled && option->state & QStyle::State_HasFocus) {
    option->state = option->state & ~QStyle::State_HasFocus;
  }
}

void Kryvo::FileListDelegate::paint(QPainter* painter,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
  const int column = index.column();

  switch (column) {
    case 0: {
      QStyleOptionViewItem elidedOption = option;
      elidedOption.textElideMode = Qt::ElideLeft;

      QStyledItemDelegate::paint(painter, elidedOption, index);

      break;
    }
    case 1: {
      QStyleOptionViewItem elidedOption = option;
      elidedOption.textElideMode = Qt::ElideLeft;

      QStyledItemDelegate::paint(painter, elidedOption, index);

      break;
    }
    case 2: {
      // Set up a QStyleOptionProgressBar to mimic the environment of a progress
      // bar
      QStyleOptionProgressBar progressBarOption;
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

      // Set the progress and text values of the style option
      const int progress =
          index.model()->data(index, Qt::DisplayRole).toInt();
      progressBarOption.progress = progress < 0 ? 0 : progress;
      progressBarOption.text = QString{"%1%"}.arg(progressBarOption.progress);

      // Draw the progress bar onto the view
      QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                         &progressBarOption,
                                         painter);
      break;
    }
    case 3: {
      QStyleOptionButton buttonOption;
      buttonOption.state = QStyle::State_Enabled;
      buttonOption.direction = QApplication::layoutDirection();
      buttonOption.rect = QRect{option.rect.x(),
                                option.rect.y(),
                                option.rect.width(),
                                option.rect.height()};
      buttonOption.fontMetrics = QApplication::fontMetrics();
      buttonOption.features = QStyleOptionButton::Flat;
      const QIcon closeIcon{QStringLiteral(":/images/closeFileIcon.png")};
      buttonOption.icon = closeIcon;
      const auto iconDimension =
        qMax(qMin(option.rect.width() / 2, option.rect.height() / 2), 6) -
        4;

      const QSize iconSize = QSize{iconDimension, iconDimension};
      buttonOption.iconSize = iconSize;

      QApplication::style()->drawControl(QStyle::CE_PushButton,
                                         &buttonOption,
                                         painter);
      break;
    }
  }
}

bool Kryvo::FileListDelegate::editorEvent(QEvent* event,
                                          QAbstractItemModel* model,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) {
  if (3 == index.column()) {
    if (QEvent::MouseButtonRelease == event->type() ||
        QEvent::MouseButtonDblClick == event->type()) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

      if (Qt::LeftButton == mouseEvent->button() &&
          option.rect.contains(mouseEvent->pos())) {
        emit removeRow(index);
      }
    }
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}