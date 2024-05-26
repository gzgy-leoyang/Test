#ifndef DATA_MODLE_H
#define DATA_MODLE_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QAbstractListModel>

typedef struct {
    QString left_str;
    QString left_color;
    QString right_str;
    QString right_color;
}Data_item_t;

class Data_element : public QObject
{
    Q_OBJECT
public:
    Data_element(QString _left,
            QString _right,
            QString _color,
            QString _left_color,
            QObject* parent=0);

    QString left_str;
    QString left_color;
    QString right_str;
    QString right_color;
public:
};
////////////////////////////////////////////

class Data_model : public QAbstractListModel
{
    Q_OBJECT
public:
    Data_model(QObject *parent=0);
    /*必须实现的虚函数 供QML引擎调用*/
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames()  const override;
    bool pushData( Data_element* _entry );
    //void refresh( const int& _index ,const QString& _sta);
    //void refresh( const int& _index ,
    //         const QString& _right,
    //         const QString& _left,
    //         const QString& _color);
    //void refresh_color( const int& _index,
    //                    const QString& _color);
    //void refresh_left_color( const int& _index,
    //                    const QString& _color);

    //void refresh_left_right( const int&     _index,
    //                         const QString& _left,
    //                         const QString& _right);

    void refresh_highlight( const int& _index,
                            const QString& _highlight);
    void refresh( const int& _index ,
                  const int& _sta);

    void refresh( const int&     _index ,
                  const QString& _right);

    void refresh( const int&     _index ,
                  const QString& _left,
                  const QString& _right);

    void refresh( const int&     _index ,
                  const QString& _left,
                  const QString& _right,
                  const QString& _right_color);

    void refresh( const int&     _index ,
                  const QString& _left,
                  const QString& _left_color,
                  const QString& _right,
                  const QString& _right_color);

public:
    QList< Data_element* > m_params_list;//被封装的数组
private:
};
////////////////////////////////////////////

#endif // DATA_MODLE_H
