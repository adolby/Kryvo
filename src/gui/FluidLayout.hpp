#ifndef KRYVOS_GUI_FLUIDLAYOUT_HPP_
#define KRYVOS_GUI_FLUIDLAYOUT_HPP_

#include "src/gui/flowlayout.h"
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
   * \brief FluidLayout Constructs a fluid layout with a parent
   * \param parent Widget parent
   * \param margin Integer representing margin
   * \param hSpacing Integer representing horizontal spacing
   * \param vSpacing Integer representing vertical spacing
   */
  explicit FluidLayout(QWidget* parent, int margin = -1,
                       int hSpacing = -1, int vSpacing = -1);

  /*!
   * \brief FluidLayout Constructs a fluid layout
   * \param margin Integer representing margin
   * \param hSpacing Integer representing horizontal spacing
   * \param vSpacing Integer representing vertical spacing
   */
  explicit FluidLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);

  /*!
   * \brief heightForWidth Calculates the height for width accounting for line
   * count
   * \return Width
   */
  int heightForWidth(int width) const;

  /*!
   * \brief setGeometry Sets the geometry for this layout manager to the
   * parameter rectangle rect
   * \param rect New geometry rectangle
   */
  void setGeometry(const QRect& rect);

 protected:
  /*!
   * \brief doLayout Calculates the layout geometry and updates the line count
   * accordingly, which allows the layout manager to return an accurate size
   * of its children
   * \param rect New geometry rectangle
   * \param testOnly Boolean indicating calculation only
   * \return Height in pixels
   */
  int doLayout(const QRect& rect, const bool testOnly) const;
};

#endif // KRYVOS_GUI_FLUIDLAYOUT_HPP_
