#ifndef KRYVOS_GUI_FILESETMODEL_HPP_
#define KRYVOS_GUI_FILESETMODEL_HPP_

#include <QtCore/QAbstractListModel>
#include <memory>

class FileSetModel : public QAbstractListModel
{
  Q_OBJECT

 public:
  FileSetModel(const QStringList& stringSet, QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole);
  bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
  bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

 private:
  QStringList stringSet;
};

#endif // KRYVOS_GUI_FILESETMODEL_HPP_
