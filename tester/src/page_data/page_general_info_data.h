#ifndef PAGE_GENERAL_INFO_DATA_H
#define PAGE_GENERAL_INFO_DATA_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QAbstractListModel>
#include <QTimer>

#include "misc/data_model.h"


class Page_general_info_data : public QObject
{
    Q_OBJECT
public:
    explicit Page_general_info_data(QObject *parent = nullptr);

    Q_PROPERTY( Data_model* _model  READ get_model)
    Data_model* get_model(void) const { return m_model ;}

    Data_model* m_model ;

    Data_item_t m_menu[15] = {
        { .left_str  = "执行脚本",
          .left_color = "#2b2a4c",
          .right_str = "等待加载...",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "本次启动时刻",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "测试循环计数",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "累计循环计数",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "测试时长",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "累计测试时长",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },

        { .left_str  = "CAN_0 发送帧率",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "CAN_0 接收帧率",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "CAN_0 总线负载",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "CAN_1 发送帧率",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "CAN_1 接收帧率",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "CAN_1 总线负载",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },

        { .left_str  = "ACC控制状态",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "BAT控制状态",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
        { .left_str  = "剩余随机时间",
          .left_color = "#2b2a4c",
          .right_str = "--",
          .right_color = "#2b2a4c",
        },
    };
signals:

public slots:
  void timeout_slot(void);
private:
  QTimer* m_tim;
};

#endif // MAIN_DATA_H
