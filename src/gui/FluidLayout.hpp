#ifndef KRYVOS_GUI_FLUIDLAYOUT_HPP_
#define KRYVOS_GUI_FLUIDLAYOUT_HPP_

#include "gui/flowlayout.h"
#include <QtWidgets/QWidget>

class FluidLayout : public FlowLayout
{
 public:
  explicit FluidLayout(QWidget* parent, int margin = -1,
                       int hSpacing = -1, int vSpacing = -1);
  explicit FluidLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);

  int heightForWidth(int) const;
  QSize minimumSize() const;
  void setGeometry(const QRect& rect);

 protected:
  void doLayout(const QRect& rect);
  int checkLayout(const QRect& rect) const;

  int lineCount;
};

#endif // KRYVOS_GUI_FLUIDLAYOUT_HPP_
