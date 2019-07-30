#include "models/FileListModel.hpp"

class Kryvo::FileListModelPrivate {
  Q_DISABLE_COPY(FileListModelPrivate)

 public:
  FileListModelPrivate();

  std::vector<FileItem> files;
};

Kryvo::FileListModelPrivate::FileListModelPrivate() {
}

Kryvo::FileListModel::FileListModel(QObject* parent)
  : QAbstractListModel(parent),
    d_ptr(std::make_unique<FileListModelPrivate>()){
}

Kryvo::FileListModel::~FileListModel() = default;

int Kryvo::FileListModel::rowCount(const QModelIndex& parent) const {
  Q_D(const FileListModel);

  Q_UNUSED(parent);

  return static_cast<int>(d->files.size());
}

QVariant Kryvo::FileListModel::data(const QModelIndex& index, int role) const {
  Q_D(const FileListModel);

  if (!index.isValid()) {
    return QVariant();
  }

  const int size = static_cast<int>(d->files.size());

  if (index.row() < 0 || index.row() >= size) {
    return QVariant();
  }

  const FileItem& el = d->files.at(index.row());

  if (ProgressRole == role) {
    return el.progress();
  } else if (TaskRole == role) {
    return el.task();
  } else if (FileNameRole == role) {
    return el.fileName();
  }

  return QVariant();
}

bool Kryvo::FileListModel::setData(const QModelIndex& index,
                                   const QVariant& value,
                                   int role) {
  Q_D(FileListModel);

  const int size = static_cast<int>(d->files.size());

  if (index.row() < 0 || index.row() >= size) {
    return false;
  }

  const int row = index.row();

  FileItem item = d->files.at(row);

  if (TaskRole == role) {
    item.setTask(value.toString());
  } else if (ProgressRole == role) {
    item.setProgress(value.toInt());
  }

  d->files[row] = item;

  emit dataChanged(index, index);

  return true;
}

std::vector<Kryvo::FileItem> Kryvo::FileListModel::fileListData() const {
  Q_D(const FileListModel);
  return d->files;
}

Kryvo::FileItem Kryvo::FileListModel::item(const int id) const {
  Q_D(const FileListModel);
  return d->files.at(id);
}

void Kryvo::FileListModel::init(const std::vector<FileItem>& fileList) {
  Q_D(FileListModel);

  beginResetModel();

  d->files = fileList;

  endResetModel();
}

void Kryvo::FileListModel::appendRow(const FileItem& item) {
  Q_D(FileListModel);

  const bool itemInModel =
    (std::find(d->files.begin(), d->files.end(), item.fileName()) !=
     d->files.end());

  if (itemInModel) { // Guard against adding duplicate items to model
    return;
  }

  beginInsertRows(index(d->files.size()),
                  static_cast<int>(d->files.size()),
                  static_cast<int>(d->files.size()));

  d->files.push_back(item);

  endInsertRows();
}

void Kryvo::FileListModel::updateRow(const QString& fileName,
                                     const QString& task,
                                     const int progressValue) {
  Q_D(FileListModel);

  const ptrdiff_t pos = std::distance(d->files.begin(),
                                      std::find(d->files.begin(),
                                                d->files.end(),
                                                fileName));

  setData(index(pos), task, TaskRole);
  setData(index(pos), progressValue, ProgressRole);
}

void Kryvo::FileListModel::clear() {
  Q_D(FileListModel);

  beginResetModel();

  d->files.clear();

  endResetModel();
}

QHash<int, QByteArray> Kryvo::FileListModel::roleNames() const {
  QHash<int, QByteArray> roles;

  roles[FileNameRole] = QByteArrayLiteral("fileName");
  roles[TaskRole] = QByteArrayLiteral("task");
  roles[ProgressRole] = QByteArrayLiteral("progress");

  return roles;
}
