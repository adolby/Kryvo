#include "src/gui/FluidLayout.hpp"

FluidLayout::FluidLayout(QWidget* parent, int margin,
                         int hSpacing, int vSpacing)
  : FlowLayout{parent, margin, hSpacing, vSpacing}, lineCount{1}
{}

FluidLayout::FluidLayout(int margin, int hSpacing, int vSpacing)
  : FlowLayout{margin, hSpacing, vSpacing}, lineCount{1}
{}

int FluidLayout::heightForWidth(int width) const
{
  const auto height = checkLayout(QRect{0, 0, width, 0});

  return height;
}

void FluidLayout::setGeometry(const QRect& rect)
{
  QLayout::setGeometry(rect);
  doLayout(rect);
}

QSize FluidLayout::minimumSize() const
{
  auto largestChildSize = QSize{};

  for (const auto& item : itemList)
  {
    largestChildSize = largestChildSize.expandedTo(item->sizeHint());
  }

  auto left = 0;
  auto top = 0;
  auto right = 0;
  auto bottom = 0;
  getContentsMargins(&left, &top, &right, &bottom);
  const auto margin = left;

  const auto margins = QSize{2 * margin,
                             (lineCount + 1) * margin};
  const auto minSize = QSize{largestChildSize.width(),
                             lineCount * largestChildSize.height()} +
                       margins;

  return minSize;
}

void FluidLayout::doLayout(const QRect& rect)
{
  auto left = 0;
  auto top = 0;
  auto right = 0;
  auto bottom = 0;
  getContentsMargins(&left, &top, &right, &bottom);
  auto effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  auto x = effectiveRect.x();
  auto y = effectiveRect.y();
  int lineHeight = 0;

  auto testLineCount = 1;

  for (const auto& item : itemList)
  {
    auto wid = item->widget();

    auto spaceX = horizontalSpacing();
    if (spaceX == -1)
    {
      spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Horizontal);
    }

    auto spaceY = verticalSpacing();
    if (spaceY == -1)
    {
      spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Vertical);
    }

    auto nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
    {
      x = effectiveRect.x();
      y = y + lineHeight + spaceY;
      nextX = x + item->sizeHint().width() + spaceX;
      lineHeight = 0;

      ++testLineCount;
    }

    item->setGeometry(QRect{QPoint{x, y}, item->sizeHint()});

    x = nextX;
    lineHeight = qMax(lineHeight, item->sizeHint().height());
  }

  if (testLineCount != lineCount)
  {
    lineCount = testLineCount;
    invalidate();
  }
}

int FluidLayout::checkLayout(const QRect& rect) const
{
  auto left = 0;
  auto top = 0;
  auto right = 0;
  auto bottom = 0;
  getContentsMargins(&left, &top, &right, &bottom);
  auto effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  auto x = effectiveRect.x();
  auto y = effectiveRect.y();
  auto lineHeight = 0;

  for (const auto& item : itemList)
  {
    auto wid = item->widget();

    auto spaceX = horizontalSpacing();
    if (spaceX == -1)
    {
      spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Horizontal);
    }

    auto spaceY = verticalSpacing();
    if (spaceY == -1)
    {
      spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton,
                                           QSizePolicy::PushButton,
                                           Qt::Vertical);
    }

    auto nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
    {
      x = effectiveRect.x();
      y = y + lineHeight + spaceY;
      nextX = x + item->sizeHint().width() + spaceX;
      lineHeight = 0;
    }

    x = nextX;
    lineHeight = qMax(lineHeight, item->sizeHint().height());
  }

  return y + lineHeight - rect.y() + bottom;
}
