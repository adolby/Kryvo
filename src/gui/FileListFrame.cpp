#include "src/gui/FileListFrame.hpp"
#include "src/gui/FileListDelegate.hpp"
#include "src/utility/pimpl_impl.h"
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QStringRef>
#include <QStringBuilder>
#include <QString>

#include <QDebug>

#if defined(_MSC_VER)
#include "src/utility/make_unique.h"
#endif

class Kryvos::FileListFrame::FileListFramePrivate {
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

Kryvos::FileListFrame::FileListFrame(QWidget* parent)
  : QFrame{parent} {
  // File list header
  const QStringList headerList = {tr("File"), tr("Task"), tr("Progress"),
                                  tr("Remove")};
  m->fileListModel->setHorizontalHeaderLabels(headerList);

  m->fileListView = new QTableView{this};
  m->fileListView->setModel(m->fileListModel.get());
  m->fileListView->setShowGrid(false);
  m->fileListView->verticalHeader()->hide();
  m->fileListView->horizontalHeader()->hide();
  m->fileListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m->fileListView->setFrameShape(QFrame::NoFrame);

  QHeaderView* header = m->fileListView->horizontalHeader();
  header->setStretchLastSection(false);
  header->setDefaultSectionSize(130);
  header->setSectionResizeMode(QHeaderView::Fixed);

  // Custom delegate paints progress bar and file close button for each file
  auto delegate = new FileListDelegate{this};
  m->fileListView->setItemDelegate(delegate);
  m->fileListView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

  auto fileListLayout = new QVBoxLayout{this};
  fileListLayout->addWidget(m->fileListView);
  fileListLayout->setContentsMargins(0, 0, 0, 0);

  connect(delegate, &FileListDelegate::removeRow,
          this, &FileListFrame::removeFileFromModel);
}

Kryvos::FileListFrame::~FileListFrame() {
}

QStandardItem* Kryvos::FileListFrame::item(const int row) {
  Q_ASSERT(m->fileListModel);

  return m->fileListModel->item(row, 0);
}

int Kryvos::FileListFrame::rowCount() const {
  Q_ASSERT(m->fileListModel);

  return m->fileListModel->rowCount();
}

void Kryvos::FileListFrame::clear() {
  Q_ASSERT(m->fileListModel);

  m->fileListModel->clear();

  // File list header
  const QStringList headerList = {tr("File"), tr("Task"), tr("Progress"),
                                  tr("Remove")};
  m->fileListModel->setHorizontalHeaderLabels(headerList);

  QHeaderView* header = m->fileListView->horizontalHeader();
  header->setStretchLastSection(false);
  header->setDefaultSectionSize(130);
  m->fileListView->setColumnWidth(0, this->width() * 0.6);
  m->fileListView->setColumnWidth(1, this->width() * 0.15);
  m->fileListView->setColumnWidth(2, this->width() * 0.2);
  m->fileListView->setColumnWidth(3, this->width() * 0.04);
}

void Kryvos::FileListFrame::updateProgress(const QString& path,
                                           const QString& task,
                                           const qint64 percent) {
  Q_ASSERT(m->fileListModel);

  qDebug() << path << " " << task << " " << percent;

  if (!path.isEmpty()) {
    const QList<QStandardItem*>& items = m->fileListModel->findItems(path);

    if (items.size() > 0) {
      auto item = items[0];

      if (item) {
        const auto index = item->row();

        auto taskItem = m->fileListModel->item(index, 1);
        if (taskItem) {
          taskItem->setData(task, Qt::DisplayRole);
        }

        auto progressItem = m->fileListModel->item(index, 2);
        if (progressItem) {
          progressItem->setData(percent, Qt::DisplayRole);
        }
      }
    }
  }
}

void Kryvos::FileListFrame::addFileToModel(const QString& path) {
  QFileInfo fileInfo{path};

  if (fileInfo.exists() && fileInfo.isFile()) {
    // If the file exists, add it to the model
    auto pathItem = new QStandardItem{path};
    pathItem->setDragEnabled(false);
    pathItem->setDropEnabled(false);
    pathItem->setEditable(false);
    pathItem->setSelectable(false);
    pathItem->setToolTip(path);
    QVariant pathVariant = QVariant::fromValue(path);
    pathItem->setData(pathVariant);

    auto taskItem = new QStandardItem{};
    pathItem->setDragEnabled(false);
    pathItem->setDropEnabled(false);
    pathItem->setEditable(false);
    pathItem->setSelectable(false);

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

    const QList<QStandardItem*> items = {pathItem, taskItem, progressItem,
                                         closeFileItem};

    // Search to see if this item is already in the model
    auto addNewItem = true;

    const auto rowCount = m->fileListModel->rowCount();
    for (auto row = 0; row < rowCount; ++row) {
      const auto& testItem = m->fileListModel->item(row, 0);

      if (testItem->data().toString() == pathItem->data().toString()) {
        addNewItem = false;
      }
    }

    if (addNewItem) { // Add the item to the model if it's new
      m->fileListModel->appendRow(items);
    }
  } // End if file exists and is a file
}

void Kryvos::FileListFrame::removeFileFromModel(const QModelIndex& index) {
  Q_ASSERT(m->fileListModel);

  auto testItem = m->fileListModel->item(index.row(), 0);

  // Signal that this file shouldn't be encrypted or decrypted
  emit stopFile(testItem->data().toString());

  // Remove row from model
  m->fileListModel->removeRow(index.row());
}

void Kryvos::FileListFrame::resizeEvent(QResizeEvent* event) {
  Q_UNUSED(event);
  Q_ASSERT(m->fileListView);

  const auto width = this->width();

  m->fileListView->setColumnWidth(0, width * 0.6);
  m->fileListView->setColumnWidth(1, width * 0.15);
  m->fileListView->setColumnWidth(2, width * 0.2);
  m->fileListView->setColumnWidth(3, width * 0.04);
}

Kryvos::FileListFrame::FileListFramePrivate::FileListFramePrivate()
  : fileListModel{std::make_unique<QStandardItemModel>()},
    fileListView{nullptr} {
}
