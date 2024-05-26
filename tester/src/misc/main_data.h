#ifndef MAIN_DATA_H
#define MAIN_DATA_H

#include <QObject>
#include <QTimer>

#include "data_model.h"
#include "uds_tester.h"
//#include "tester/tester.h"
//#include "main.h"




//typedef struct {
//  int total_cycle ;
//  int total_second;
//} Test_record_t;

class Main_data : public QObject
{
    Q_OBJECT
public:
  explicit Main_data(QObject *parent = nullptr);

  Q_PROPERTY( int _menu_index  READ get_menu_index WRITE set_menu_index NOTIFY menu_index_changed)
  int get_menu_index(void) const;
  void set_menu_index(const int _index);

  Q_PROPERTY( Data_model* _model  READ get_model NOTIFY model_changed)
  Data_model* get_model(void) const { return m_model ;}

  Q_PROPERTY( QString _datetime READ get_datetime NOTIFY datetime_changed )
  QString get_datetime(void) const { return QString("本次启动: ")+m_start_date_time ;}

  Q_PROPERTY( int _run READ get_run  NOTIFY run_changed )
  int get_run( void ) const { return m_run;}

  Q_INVOKABLE void run_pause_clicked(void);
  Q_INVOKABLE void clean_clicked(void);
  Q_INVOKABLE void dev_record_clicked(int _dev_index);
  Q_INVOKABLE void devs_record_clicked(void);

  Q_INVOKABLE void manual_ctrl_click(int _val);

  Data_model* m_model ;

  Data_item_t m_menu[ 21 ] = {
      { .left_str  = "状态",
        .left_color = "#6699CC",
        .right_str = "等待开始",
        .right_color = "#6699CC",
      },
      { .left_str  = "设置",
        .left_color = "#6699CC",
        .right_str = "",
        .right_color = "#6699CC",
      },
      { .left_str  = "被测设备[0]",
        .left_color = "#6699CC",
        .right_str = "Normal",
        .right_color = "#6699CC",
      },
  };

signals:
    void menu_index_changed ();
    void model_changed ();
    void datetime_changed ();
    void run_changed ();
    void run_tick( int _flag,int _acc,int _bat );

public slots:
    void timeout_slot(void);
private:
    //int record_to_file( const std::string _file,
    //                    const std::string _line);
    //int device_record( const int _dev_index );
    //int device_record( const int _dev_index,const int _type=0 );
    //int all_devices_record(void);
    //int test_record(void);
    //int init_test_sys();

    QTimer*    m_tim;
    int        m_run;
    int        m_menu_index;
    QString    m_start_date_time;
    int        m_devs_record_preoid_in_sec;
    int        m_test_record_preoid_in_sec;
};

#endif // MAIN_DATA_H
