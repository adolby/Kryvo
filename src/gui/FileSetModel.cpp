#include "FileSetModel.hpp"
#include <QStringList>

/*!
 * \brief FileSetModel::FileSetModel
 * \param fileSet
 * \param parent
 */
FileSetModel::FileSetModel(const QStringList& stringSet, QObject* parent) :
  QAbstractListModel{parent}
{
  this->stringSet = stringSet;
}

/*!
 * \brief FileSetModel::rowCount Returns the number of rows in the model.
 * \param parent
 * \return
 */
int FileSetModel::rowCount(const QModelIndex& /*parent*/) const
{
  return stringSet.count();
}

/*!
 * \brief FileSetModel::data Returns the data in the model at the specified
 * index.
 * \param index The QModelIndex reference from where the data will be returned.
 * \param role The integer representing the role.
 * \return
 */
QVariant FileSetModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() < 0 || index.row() >= stringSet.size())
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    return stringSet.at(index.row());
  else
    return QVariant();
}

/*!
 * \brief FileSetModel::headerData Returns the header data for the model.
 * \param section The integer representing the section.
 * \param orientation The enum representing the orientation.
 * \param role The integer representing the role.
 * \return
 */
QVariant FileSetModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
    return QString("Column %1").arg(section);
  else
    return QString("Row %1").arg(section);
}

/*!
 * \brief FileSetModel::flags
 * \param index
 * \return
 */
Qt::ItemFlags FileSetModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;

  return QAbstractListModel::flags(index) | Qt::ItemIsEditable |
      Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

/*!
 * \brief FileSetModel::setData
 * \param index
 * \param value
 * \param role
 * \return
 */
bool FileSetModel::setData(const QModelIndex& index,
                             const QVariant& value,
                             int role)
{
  if (index.isValid() && role == Qt::EditRole)
  {
    stringSet.replace(index.row(), value.toString());
    emit dataChanged(index, index);
    return true;
  }

  return false;
}

/*!
 * \brief FileSetModel::insertRows
 * \param position
 * \param rows
 * \param parent
 * \return
 */
bool FileSetModel::insertRows(int position,
                                int rows,
                                const QModelIndex& parent)
{
  beginInsertRows(QModelIndex(), position, position + rows - 1);

  for (int row = 0; row < rows; ++row)
  {
    stringSet.insert(position, "");
  }

  endInsertRows();
  return true;
}

/*!
 * \brief FileSetModel::removeRows
 * \param position
 * \param rows
 * \param parent
 * \return
 */
bool FileSetModel::removeRows(int position,
                                int rows,
                                const QModelIndex& parent)
{
  beginRemoveRows(QModelIndex(), position, position + rows - 1);

  for (int row = 0; row < rows; ++row)
  {
    stringSet.removeAt(position);
  }

  endRemoveRows();
  return true;
}

void FileSetModel::appendRow()
{
  beginInsertRows(QModelIndex(), stringSet.length(), stringSet.length());



  endInsertRows();
}
