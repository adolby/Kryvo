#include "FileListDelegate.hpp"
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QEvent>

namespace Kryvo {

FileListDelegate::FileListDelegate(QObject* parent)
  : QStyledItemDelegate(parent) {
}

void FileListDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  const int column = index.column();

  switch (column) {
    case 0: {
      QStyleOptionViewItem updatedOption = option;
      updatedOption.textElideMode = Qt::ElideLeft;

#if defined(Q_OS_MACOS)
      updatedOption.font.setPointSize(20);
#else
      updatedOption.font.setPointSize(16);
#endif

      QStyledItemDelegate::paint(painter, updatedOption, index);
      break;
    }

    case 1: {
      QStyleOptionViewItem updatedOption = option;
      updatedOption.textElideMode = Qt::ElideRight;

#if defined(Q_OS_MACOS)
      updatedOption.font.setPointSize(20);
#else
      updatedOption.font.setPointSize(16);
#endif

      QStyledItemDelegate::paint(painter, updatedOption, index);
      break;
    }

    case 2: {
      const QVariant progressVariant = index.data(Qt::DisplayRole);
      const int progress = qvariant_cast<int>(progressVariant);
      const int progressAbsolute = progress < 0 ? 0 : progress;

      // Set up a QStyleOptionProgressBar to mimic the environment of a progress
      // bar
      QStyleOptionProgressBar progressBarOption;
      progressBarOption.state = option.state | QStyle::State_Horizontal;
      progressBarOption.direction = painter->layoutDirection();
      progressBarOption.rect = QRect(option.rect.x(),
                                     option.rect.y() + 1,
                                     option.rect.width(),
                                     option.rect.height() - 1);
      progressBarOption.fontMetrics = QFontMetrics(QApplication::font());
      progressBarOption.minimum = 0;
      progressBarOption.maximum = 100;
      progressBarOption.textAlignment = Qt::AlignCenter;
      progressBarOption.palette = option.palette;
      progressBarOption.textVisible = true;

      // Set the progress and text values of the style option
      progressBarOption.progress = progressAbsolute;
      progressBarOption.text = QStringLiteral("%1%").arg(progressAbsolute);

#if defined(Q_OS_MACOS)
      painter->save();
      painter->translate(progressBarOption.rect.left(),
                         progressBarOption.rect.top());
      progressBarOption.rect.moveTo(0,0);
#endif

      QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                         &progressBarOption,
                                         painter);

#if defined(Q_OS_MACOS)
      painter->restore();
#endif

      break;
    }

    case 3: {
      QStyleOptionButton buttonOption;
      buttonOption.state = QStyle::State_Enabled;
      buttonOption.direction = painter->layoutDirection();
      buttonOption.rect = option.rect;
      buttonOption.fontMetrics = painter->fontMetrics();
      buttonOption.features = QStyleOptionButton::Flat;

      const QIcon closeIcon(QStringLiteral(":/images/closeFileIcon.png"));
      buttonOption.icon = closeIcon;

      const int iconDimension =
        qMax(qMin(option.rect.width() / 2, option.rect.height() / 2), 6) -
        4;
      const QSize iconSize(iconDimension, iconDimension);
      buttonOption.iconSize = iconSize;

      QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption,
                                         painter);
      break;
    }

    default: {
      QStyledItemDelegate::paint(painter, option, index);
      break;
    }
  }
}

bool FileListDelegate::editorEvent(QEvent* event,
                                   QAbstractItemModel* model,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) {
  if (3 == index.column()) {
    if (QEvent::MouseButtonRelease == event->type() ||
        QEvent::MouseButtonDblClick == event->type()) {
      auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);

      if (Qt::LeftButton == mouseEvent->button() &&
          option.rect.contains(mouseEvent->pos())) {
        emit removeRow(index);
      }
    }
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

} // namespace Kryvo
