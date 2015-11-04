#ifndef KRYVOS_GUI_FILELISTFRAME_HPP_
#define KRYVOS_GUI_FILELISTFRAME_HPP_

#include "utility/pimpl.h"
#include <QFrame>
#include <QStandardItem>

/*!
 * \brief The FileListFrame class contains the file list that displays the file
 * name, progress bar, and close button.
 */
class FileListFrame : public QFrame {
  Q_OBJECT

 public:
  /*!
   * \brief FileListFrame Constructs a file list frame, which displays file
   * information and allows users to remove file entries.
   * \param parent Widget parent of this file list frame.
   */
  explicit FileListFrame(QWidget* parent = nullptr);

  /*!
   * \brief ~FileListFrame Destroys a file list frame.
   */
  virtual ~FileListFrame();

 signals:
  /*!
   * \brief stopFile Emitted when the user clicks a remove file button.
   */
  void stopFile(const QString& fileName);

 public slots:
  /*!
   * \brief addFileToModel Adds a file to the model that represents the list
   * to be encrypted/decrypted.
   * \param path String representing the path to a file
   */
  void addFileToModel(const QString& path);

  /*!
   * \brief removeFileFromModel Removes the file name at the input index in the
   * model.
   * \param index The index of the file name to remove from the model
   */
  void removeFileFromModel(const QModelIndex& index);

 public:
  /*!
   * \brief item Returns a standard item at the input index in the file list
   * model.
   * \param row Integer representing the file list model row
   * \return Standard item taken from specified index in the file list model
   */
  QStandardItem* item(const int row);

  /*!
   * \brief rowCount Returns the number of rows in the file list model.
   * \return Number of rows as an integer in the file list model
   */
  int rowCount() const;

  /*!
   * \brief clear Clears the file list model.
   */
  void clear();

  /*!
   * \brief updateProgress Executed when the cipher operation progress is
   * updated. Updates the progress bar for the item at the specified index.
   * \param index Index as an integer in the file list toupdate
   * \param percent Current progress as an integer percentage
   */
  void updateProgress(const QString& path, const qint64 percent);

 protected:
  void resizeEvent(QResizeEvent* event);

 private:
  class FileListFramePrivate;
  pimpl<FileListFramePrivate> m;
};

#endif // KRYVOS_GUI_FILELISTFRAME_HPP_
