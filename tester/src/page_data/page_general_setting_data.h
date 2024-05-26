#ifndef PAGE_GENERAL_SETTING_DATA_H
#define PAGE_GENERAL_SETTING_DATA_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QAbstractListModel>
#include <QTimer>

#include "misc/uds_tester.h"
#include "misc/data_model.h"
//#include "misc/tester/tester.h"

class Page_general_setting_data : public QObject
{
    Q_OBJECT
public:

  Q_PROPERTY( Data_model* _model  READ get_model)
  Data_model* get_model(void) const { return m_model ;}

  Data_model* m_model ;

  Data_item_t m_menu[50] = {
      { .left_str  = "动作",
        .left_color = "gray",
        .right_str = "时刻",
        .right_color = "gray",
      },
  };
  explicit Page_general_setting_data(QObject *parent = nullptr);

public slots:
  void timeout_slot(void);
private:
  QTimer* m_tim;

signals:
};

#endif // MAIN_DATA_H
