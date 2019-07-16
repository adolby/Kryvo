#ifndef KRYVO_FILELISTMODEL_HPP_
#define KRYVO_FILELISTMODEL_HPP_

#include "FileItem.hpp"
#include "utility/pimpl.h"
#include <QAbstractListModel>
#include <QStringList>
#include <vector>
#include <memory>

namespace Kryvo {

class FileListModelPrivate;

class FileListModel : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY(FileListModel)
  DECLARE_PRIVATE(FileListModel)

  std::unique_ptr<FileListModelPrivate> const d_ptr;

 public:
  enum FileListRoles {
    ProgressRole = Qt::UserRole + 2,
    TaskRole = Qt::UserRole + 1,
    FileNameRole = Qt::UserRole
  };

  explicit FileListModel(QObject* parent = nullptr);
  ~FileListModel() override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole) override;

  std::vector<FileItem> fileListData() const;
  FileItem item(int id) const;

 public slots:
  void init(const std::vector<FileItem>& files);
  void appendRow(const FileItem& item);
  void updateRow(const QString& fileName, const QString& task,
                 int progressValue);
  void clear();

 protected:
  QHash<int, QByteArray> roleNames() const override;
};

} // namespace Kryvo

#endif // KRYVO_FILELISTMODEL_HPP_
