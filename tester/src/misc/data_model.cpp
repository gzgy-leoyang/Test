#include "data_model.h"

/// >>> 模型
Data_model::
Data_model(QObject *parent):
    QAbstractListModel(parent)
{
}

int Data_model::
rowCount(const QModelIndex &parent) const
{
    return m_params_list.count();//返回私有列表数据量
}

QVariant Data_model::
data(const QModelIndex &index, int role) const
{
    Data_element* op = m_params_list.at(index.row());
    if (role == Qt::UserRole)
        return op->left_str;

    if (role == Qt::UserRole+1)
        return op->right_str;

    if (role == Qt::UserRole+2)
        return op->right_color;

    if (role == Qt::UserRole+3)
        return op->left_color;
}

QHash<int, QByteArray> Data_model::
roleNames() const
{
    QHash<int, QByteArray>  hash;
    hash.insert(Qt::UserRole  , QByteArray("left_str"));
    hash.insert(Qt::UserRole+1, QByteArray("right_str"));
    hash.insert(Qt::UserRole+2, QByteArray("right_color"));
    hash.insert(Qt::UserRole+3, QByteArray("left_color"));
    return hash;
}

bool Data_model::
pushData(Data_element* _entry)
{
  if ( _entry == NULL ){
    return false;
  }

  if ( &m_params_list == NULL ){
    return false;
  }
  beginInsertRows( QModelIndex(), m_params_list.count(), m_params_list.count());
  m_params_list.append(_entry);
  endInsertRows();
  return true;
}

void Data_model::
refresh( const int& _index ,
         const int& _val)
{
  Data_element* op = m_params_list.at(_index);
  QModelIndex indexxx = index(_index);
  //op->completion_rate = _sta;
  op->right_str = QString("%1").arg(_val);// "Right String";
  emit QAbstractItemModel::dataChanged(indexxx,indexxx);
}

void Data_model::
refresh( const int&     _index ,
         const QString& _right)
{
  Data_element* op    = m_params_list.at(_index);
  QModelIndex indexxx = index(_index);
  op->right_str       = _right;
  emit QAbstractItemModel::dataChanged(indexxx,indexxx);
}

void Data_model::
refresh_highlight( const int&     _index,
                   const QString& _highlight)
{
  Data_element* op    = m_params_list.at(_index);
  QModelIndex indexxx = index(_index);
  op->left_color      = _highlight;
  op->right_color     = _highlight;
  emit QAbstractItemModel::dataChanged(indexxx,indexxx);
}

void Data_model::
refresh( const int&     _index ,
         const QString& _left,
         const QString& _right)
{
  Data_element* op    = m_params_list.at(_index);
  QModelIndex indexxx = index(_index);
  op->left_str        = _left;
  op->right_str       = _right;
  emit QAbstractItemModel::dataChanged(indexxx,indexxx);
}

void Data_model::
refresh( const int& _index ,
         const QString& _left,
         const QString& _right,
         const QString& _right_color)
{
  Data_element* op    = m_params_list.at(_index);
  QModelIndex indexxx = index(_index);
  op->left_str        = _left;
  op->right_str       = _right;
  op->right_color     = _right_color;
  emit QAbstractItemModel::dataChanged(indexxx,indexxx);
}

void Data_model::
refresh( const int& _index ,
         const QString& _left,
         const QString& _left_color,
         const QString& _right,
         const QString& _right_color)
{
  Data_element* op    = m_params_list.at(_index);
  QModelIndex indexxx = index(_index);
  op->left_str        = _left;
  op->left_color     =  _left_color;
  op->right_str       = _right;
  op->right_color     = _right_color;
  emit QAbstractItemModel::dataChanged(indexxx,indexxx);
}

//// >>> 数据
Data_element::
Data_element(QString _left,
        QString _left_color,
        QString _right,
        QString _right_color,
        QObject* parent):
    QObject(parent),
    left_str(_left),
    left_color(_left_color),
    right_str(_right),
    right_color(_right_color)
{
}
/// <<< 数据

/// end of code ///
