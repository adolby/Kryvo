/**
 * Kryvos - Encrypts and decrypts files.
 * Copyright (C) 2014, 2015 Andrew Dolby
 *
 * This file is part of Kryvos.
 *
 * Kryvos is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kryvos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kryvos.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "gui/FileListFrame.hpp"
#include "gui/FileListDelegate.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QScroller>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableView>
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
  FileListFramePrivate();

  /*!
   * \brief addFileToModel Adds a file to the file list model.
   * \param filePath String containing the file path.
   */
  void addFileToModel(const QString& filePath);

  std::unique_ptr<QStandardItemModel> fileListModel;
  QTableView* fileListView;
};

FileListFrame::FileListFrame(QWidget* parent)
  : QFrame{parent}, pimpl{make_unique<FileListFramePrivate>()}
{
  // File list header
  const QStringList headerList = {tr("File"), tr("Progress"),
                                  tr("Remove")};
  pimpl->fileListModel->setHorizontalHeaderLabels(headerList);

  pimpl->fileListView = new QTableView{this};
  pimpl->fileListView->setModel(pimpl->fileListModel.get());
  pimpl->fileListView->setShowGrid(false);
  pimpl->fileListView->verticalHeader()->hide();
  pimpl->fileListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  pimpl->fileListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QHeaderView* header = pimpl->fileListView->horizontalHeader();
  header->setStretchLastSection(false);
  header->setDefaultSectionSize(130);
  header->setSectionResizeMode(QHeaderView::Fixed);

  // Custom delegate paints progress bar and file close button for each file
  auto delegate = new FileListDelegate{this};
  pimpl->fileListView->setItemDelegate(delegate);

  pimpl->fileListView->setHorizontalScrollMode(QAbstractItemView::
                                               ScrollPerPixel);

  // Attach a scroller to the file list for mobile devices
  QScroller::grabGesture(pimpl->fileListView, QScroller::TouchGesture);

  // Disable overshoot; it makes interacting with small widgets harder
  auto scroller = QScroller::scroller(pimpl->fileListView);

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

  connect(delegate, &FileListDelegate::removeRow,
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

  // File list header
  const QStringList headerList = {tr("File"), tr("Progress"),
                                  tr("Remove")};
  pimpl->fileListModel->setHorizontalHeaderLabels(headerList);

  QHeaderView* header = pimpl->fileListView->horizontalHeader();
  header->setStretchLastSection(false);
  header->setDefaultSectionSize(130);
  pimpl->fileListView->setColumnWidth(0, this->width() * 0.70);
  pimpl->fileListView->setColumnWidth(1, this->width() * 0.2);
  pimpl->fileListView->setColumnWidth(2, this->width() * 0.1 - 1);
}

void FileListFrame::addFileToModel(const QString& path)
{
  QFileInfo fileInfo{path};

  if (fileInfo.exists() && fileInfo.isFile())
  { // If the file exists, add it to the model
    auto pathItem = new QStandardItem{path};
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

void FileListFrame::updateProgress(const QString& path, const qint64 percent)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  const auto items = pimpl->fileListModel->findItems(path);

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

void FileListFrame::resizeEvent(QResizeEvent* event)
{
  auto width = this->width();

  pimpl->fileListView->setColumnWidth(0, width * 0.7);
  pimpl->fileListView->setColumnWidth(1, width * 0.2);
  pimpl->fileListView->setColumnWidth(2, width * 0.1 - 1);
}

FileListFrame::FileListFramePrivate::FileListFramePrivate()
  : fileListModel{make_unique<QStandardItemModel>()}, fileListView{nullptr}
{}
