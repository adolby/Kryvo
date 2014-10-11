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

#include "gui/FileListFrame.hpp"
#include "gui/Delegate.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QScroller>
#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QStandardItemModel>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QStringRef>
#include <QtCore/QStringBuilder>
#include <QtCore/QString>

class FileListFrame::FileListFramePrivate {
 public:
  /*!
   * \brief FileListFramePrivate Constructs the FileListFrame private
   * implementation.
   */
  explicit FileListFramePrivate();

  /*!
   * \brief addFileToModel Adds a file to the file list model.
   * \param filePath String containing the file path.
   */
  void addFileToModel(const QString& filePath);

  std::unique_ptr<QStandardItemModel> fileListModel;
  QTableView* fileListView;
};

FileListFrame::FileListFrame(QWidget* parent) :
  QFrame{parent}, pimpl{make_unique<FileListFramePrivate>()}
{
  // File list
  pimpl->fileListModel->setHeaderData(0, Qt::Horizontal, tr("Files"));
  pimpl->fileListModel->setHeaderData(1, Qt::Horizontal, tr("Progress"));
  pimpl->fileListModel->setHeaderData(2, Qt::Horizontal, tr("Remove file"));

  const QStringList headerList = {tr("Files"), tr("Progress"),
                                  tr("Remove file")};
  pimpl->fileListModel->setHorizontalHeaderLabels(headerList);

  pimpl->fileListView = new QTableView{this};
  pimpl->fileListView->setModel(pimpl->fileListModel.get());

  QHeaderView* header = pimpl->fileListView->horizontalHeader();
  header->setStretchLastSection(false);
  header->hide();

  pimpl->fileListView->verticalHeader()->hide();
  pimpl->fileListView->setShowGrid(false);

  // Custom delegate paints progress bar and file close button for each file
  auto delegate = new Delegate{this};
  pimpl->fileListView->setItemDelegate(delegate);

  pimpl->fileListView->setHorizontalScrollMode(QAbstractItemView::
                                                ScrollPerPixel);

  // Attach a scroller to the file list
  QScroller::grabGesture(pimpl->fileListView,
                         QScroller::TouchGesture);

  // Disable overshoot; it makes interacting with small widgets harder
  QScroller* scroller = QScroller::scroller(pimpl->fileListView);

  QScrollerProperties properties = scroller->scrollerProperties();

  QVariant overshootPolicy = QVariant::fromValue
                              <QScrollerProperties::OvershootPolicy>
                              (QScrollerProperties::OvershootAlwaysOff);

  properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy,
                             overshootPolicy);
  properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy,
                             overshootPolicy);

  scroller->setScrollerProperties(properties);

  auto fileListLayout = new QVBoxLayout{this};
  fileListLayout->addWidget(pimpl->fileListView);
  fileListLayout->setContentsMargins(0, 0, 0, 0);
  fileListLayout->setSpacing(0);

  connect(delegate, &Delegate::removeRow,
          this, &FileListFrame::removeFileFromModel);
}

FileListFrame::~FileListFrame() {}

QStandardItem* FileListFrame::item(int row) const
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  return pimpl->fileListModel->item(row, 0);
}

int FileListFrame::rowCount() const
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  return pimpl->fileListModel->rowCount();
}

void FileListFrame::clear()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  pimpl->fileListModel->clear();
}

void FileListFrame::addFileToModel(const QString& path)
{
  QFileInfo file{path};
  QDir directory{file.dir()};

  if (file.exists() && file.isFile())
  { // If the file exists, add it to the model
    QString fileName;
    if (directory.isRoot())
    {
      fileName = file.absoluteFilePath();
    }
    else
    {
      fileName = directory.rootPath() % QLatin1String{"..."} %
                 "/" % directory.dirName() % "/" % file.fileName();
    }

    auto pathItem = new QStandardItem{fileName};
    pathItem->setDragEnabled(false);
    pathItem->setDropEnabled(false);
    pathItem->setEditable(false);
    pathItem->setSelectable(false);
    pathItem->setToolTip(path);
    QVariant pathVariant = QVariant::fromValue(path);
    pathItem->setData(pathVariant);

    auto progressItem = new QStandardItem{};
    progressItem->setDragEnabled(false);
    progressItem->setDropEnabled(false);
    progressItem->setEditable(false);
    progressItem->setSelectable(false);

    auto closeFileItem = new QStandardItem{};
    closeFileItem->setDragEnabled(false);
    closeFileItem->setDropEnabled(false);
    closeFileItem->setEditable(false);
    closeFileItem->setSelectable(false);

    const QList<QStandardItem*> items = {pathItem, progressItem, closeFileItem};

    // Search to see if this item is already in the model
    auto addNewItem = true;

    const auto rowCount = pimpl->fileListModel->rowCount();
    for (auto row = 0; row < rowCount; ++row)
    {
      auto testItem = pimpl->fileListModel->item(row, 0);

      if (testItem->data().toString() == pathItem->data().toString())
      {
        addNewItem = false;
      }
    }

    if (addNewItem)
    { // Add the item to the model if it's new
      pimpl->fileListModel->appendRow(items);
      QHeaderView* header = pimpl->fileListView->horizontalHeader();
      header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      header->setResizeContentsPrecision(0);
      header->resizeSection(1, 130);
    }
  } // End if file exists and is a file
}

void FileListFrame::removeFileFromModel(const QModelIndex& index)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  auto testItem = pimpl->fileListModel->item(index.row(), 0);

  // Signal that this file shouldn't be encrypted or decrypted
  emit stopFile(testItem->data().toString());

  // Remove row from model
  pimpl->fileListModel->removeRow(index.row());
}

void FileListFrame::updateProgress(const QString& path, qint64 percent)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  QFileInfo file{path};
  QDir directory{file.dir()};

  QString fileName;

  if (directory.isRoot())
  {
    fileName = file.absoluteFilePath();
  }
  else
  {
    fileName = directory.rootPath() % QLatin1String{"..."} %
               "/" % directory.dirName() % "/" % file.fileName();
  }

  const QList<QStandardItem*> items = pimpl->fileListModel->findItems(fileName);

  if (items.size() > 0)
  {
    auto item = items[0];

    if (item != nullptr)
    {
      const auto index = item->row();

      auto progressItem = pimpl->fileListModel->item(index, 1);

      if (progressItem != nullptr)
      {
        progressItem->setData(percent, Qt::DisplayRole);
      }
    }
  }
}

FileListFrame::FileListFramePrivate::FileListFramePrivate() :
  fileListModel{make_unique<QStandardItemModel>()}, fileListView{nullptr}
{}
