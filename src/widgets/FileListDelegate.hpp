#ifndef KRYVO_WIDGETS_FILELISTDELEGATE_HPP_
#define KRYVO_WIDGETS_FILELISTDELEGATE_HPP_

#include <QStyledItemDelegate>
#include <QSize>

namespace Kryvo {

/*!
 * \brief The FileListDelegate class augments the view to display a progress bar
 * and to remove the focus border.
 */
class FileListDelegate : public QStyledItemDelegate {
  Q_OBJECT
  Q_DISABLE_COPY(FileListDelegate)

 public:
  /*!
   * \brief FileListDelegate Constructs a delegate that augments a view
   * \param parent Object parent of this delegate
   */
  explicit FileListDelegate(QObject* parent = nullptr);

 signals:
  /*!
   * \brief removeRow Emitted when the delegate needs to remove a row from its
   * model
   * \param index Index of the row to remove
   */
  void removeRow(const QModelIndex& index);

 protected:
  /*!
   * \brief paint Paints the progress bar and the close button
   * \param painter Painter object
   * \param option Style view options object. Passed on to parent's paint
   * \param index Index of the current cell in the model to paint
   */
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  /*!
   * \brief editorEvent Event to handle mouse clicks on the remove button
   * \param event Editing event
   * \param model Model
   * \param option Style option view item
   * \param index Index of the current cell in the model
   * \return
   */
  bool editorEvent(QEvent* event, QAbstractItemModel* model,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_FILELISTDELEGATE_HPP_
