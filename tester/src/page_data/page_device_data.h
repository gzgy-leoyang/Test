#ifndef PAGE_DEVICE_DATA_H
#define PAGE_DEVICE_DATA_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QAbstractListModel>
#include <QTimer>

#include "misc/data_model.h"
#include "main.h"

class Page_device_data : public QObject
{
    Q_OBJECT
public:
    explicit Page_device_data(QObject *parent = nullptr);

    Q_PROPERTY( Data_model* _model  READ get_model)
    Data_model* get_model(void) const { return m_model ;}

    Data_model* m_model ;

    Data_item_t m_menu[_SIZE_OF_OP] = {
        { .left_str  = "Description",
          .left_color = "#2b2a4c",
          .right_str = "Parameter",
          .right_color = "#2b2a4c",
        },
    };
signals:
    void device_info_changed( const QString _str_1 ,const QString _str_2 );

public slots:
      void timeout_slot(void);
private:
  QTimer* m_tim;
};

#endif // MAIN_DATA_H
