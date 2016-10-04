#include "src/gui/FileListFrame.hpp"
#include "src/gui/FileListDelegate.hpp"
#include "src/utility/pimpl_impl.h"
#include <QScroller>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QStringRef>
#include <QStringBuilder>
#include <QString>

#if defined(_MSC_VER)
#include "src/utility/make_unique.h"
#endif

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
  : QFrame{parent}
{
  // File list header
  const QStringList headerList = {tr("File"), tr("Progress"), tr("Remove")};
  m->fileListModel->setHorizontalHeaderLabels(headerList);

  m->fileListView = new QTableView{this};
  m->fileListView->setModel(m->fileListModel.get());
  m->fileListView->setShowGrid(false);
  m->fileListView->verticalHeader()->hide();
  m->fileListView->horizontalHeader()->hide();
  m->fileListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QHeaderView* header = m->fileListView->horizontalHeader();
  header->setStretchLastSection(false);
  header->setDefaultSectionSize(130);
  header->setSectionResizeMode(QHeaderView::Fixed);

  // Custom delegate paints progress bar and file close button for each file
  auto delegate = new FileListDelegate{this};
  m->fileListView->setItemDelegate(delegate);

  m->fileListView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

  // Attach a scroller to the file list for mobile devices
  QScroller::grabGesture(m->fileListView, QScroller::TouchGesture);

  // Disable overshoot; it makes interacting with small widgets harder
  auto scroller = QScroller::scroller(m->fileListView);

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
  fileListLayout->addWidget(m->fileListView);
  fileListLayout->setContentsMargins(0, 0, 0, 0);
  fileListLayout->setSpacing(0);

  connect(delegate, &FileListDelegate::removeRow,
          this, &FileListFrame::removeFileFromModel);
}

FileListFrame::~FileListFrame() {}

QStandardItem* FileListFrame::item(const int row)
{
  Q_ASSERT(m->fileListModel);

  return m->fileListModel->item(row, 0);
}

int FileListFrame::rowCount() const
{
  Q_ASSERT(m->fileListModel);

  return m->fileListModel->rowCount();
}

void FileListFrame::clear()
{
  Q_ASSERT(m->fileListModel);

  m->fileListModel->clear();

  // File list header
  const QStringList headerList = {tr("File"), tr("Progress"), tr("Remove")};
  m->fileListModel->setHorizontalHeaderLabels(headerList);

  QHeaderView* header = m->fileListView->horizontalHeader();
  header->setStretchLastSection(false);
  header->setDefaultSectionSize(130);
  m->fileListView->setColumnWidth(0, this->width() * 0.70);
  m->fileListView->setColumnWidth(1, this->width() * 0.2);
  m->fileListView->setColumnWidth(2, this->width() * 0.1 - 1);
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

    const auto rowCount = m->fileListModel->rowCount();
    for (auto row = 0; row < rowCount; ++row)
    {
      auto testItem = m->fileListModel->item(row, 0);

      if (testItem->data().toString() == pathItem->data().toString())
      {
        addNewItem = false;
      }
    }

    if (addNewItem)
    { // Add the item to the model if it's new
      m->fileListModel->appendRow(items);
    }
  } // End if file exists and is a file
}

void FileListFrame::removeFileFromModel(const QModelIndex& index)
{
  Q_ASSERT(m->fileListModel);

  auto testItem = m->fileListModel->item(index.row(), 0);

  // Signal that this file shouldn't be encrypted or decrypted
  emit stopFile(testItem->data().toString());

  // Remove row from model
  m->fileListModel->removeRow(index.row());
}

void FileListFrame::updateProgress(const QString& path, const qint64 percent)
{
  Q_ASSERT(m->fileListModel);

  const auto items = m->fileListModel->findItems(path);

  if (items.size() > 0)
  {
    auto item = items[0];

    if (item != nullptr)
    {
      const auto index = item->row();

      auto progressItem = m->fileListModel->item(index, 1);

      if (progressItem != nullptr)
      {
        progressItem->setData(percent, Qt::DisplayRole);
      }
    }
  }
}

void FileListFrame::resizeEvent(QResizeEvent* event)
{
  Q_UNUSED(event);
  Q_ASSERT(m->fileListView);

  auto width = this->width();

  m->fileListView->setColumnWidth(0, width * 0.7);
  m->fileListView->setColumnWidth(1, width * 0.2);
  m->fileListView->setColumnWidth(2, width * 0.1 - 1);
}

FileListFrame::FileListFramePrivate::FileListFramePrivate()
  : fileListModel{std::make_unique<QStandardItemModel>()}, fileListView{nullptr}
{}
