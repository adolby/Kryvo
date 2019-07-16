#ifndef KRYVO_WIDGETS_FILELISTFRAME_HPP_
#define KRYVO_WIDGETS_FILELISTFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <QStandardItem>
#include <QFileInfo>
#include <memory>

namespace Kryvo {

class FileListFramePrivate;

/*!
 * \brief The FileListFrame class contains the file list that displays the file
 * name, progress bar, and close button.
 */
class FileListFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(FileListFrame)
  DECLARE_PRIVATE(FileListFrame)
  std::unique_ptr<FileListFramePrivate> const d_ptr;

 public:
  /*!
   * \brief FileListFrame Constructs a file list frame, which displays file
   * information and allows users to remove file entries
   * \param parent Widget parent of this file list frame
   */
  explicit FileListFrame(QWidget* parent = nullptr);

  ~FileListFrame() override;

  /*!
   * \brief item Returns a standard item at the input index in the file list
   * model
   * \param row Integer representing the file list model row
   * \return Standard item taken from specified index in the file list model
   */
  QStandardItem* item(int row);

  /*!
   * \brief rowCount Returns the number of rows in the file list model
   * \return Number of rows as an integer in the file list model
   */
  int rowCount() const;

  /*!
   * \brief clear Clears the file list model
   */
  void clear();

  /*!
   * \brief updateProgress Executed when the cipher operation progress is
   * updated. Updates the progress bar for the item at the specified index.
   * \param path File path as an index to update the file list
   * \param task Task operating on file
   * \param percent Integer representing progress as a percentage
   */
  void updateProgress(const QFileInfo& info, const QString& task,
                      qint64 percent);

 signals:
  /*!
   * \brief stopFile Emitted when the user clicks a remove file button
   */
  void stopFile(const QFileInfo& fileInfo);

 public slots:
  /*!
   * \brief addFileToModel Adds a file to the model that represents the list
   * to be encrypted/decrypted
   * \param fileInfo File
   */
  void addFileToModel(const QFileInfo& fileInfo);

  /*!
   * \brief removeFileFromModel Removes the file name at the input index in the
   * model
   * \param index The index of the file name to remove from the model
   */
  void removeFileFromModel(const QModelIndex& index);

 protected:
  /*!
   * \brief resizeEvent Executed after this widget resizes
   * \param event Event data
   */
  void resizeEvent(QResizeEvent* event) override;
};

} // namespace Kryvo

#endif // KRYVO_WIDGETS_FILELISTFRAME_HPP_
