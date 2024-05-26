import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2

import qml.custom.Page_general_setting_data 1.0

Rectangle {
  id:            page_sys_info
  width:         root_page.width_of_page
  height:        root_page.height
  anchors.right: root_page.right
  color:         root_page.background_color
  radius:7

  General_Setting_Data{
    id: setting_data
  }

  Item{
    id:                   listview_container
    anchors.top:          parent.top
    anchors.topMargin:    5

    anchors.bottom:       parent.bottom
    anchors.bottomMargin: 5
    anchors.left:         parent.left
    anchors.leftMargin:   5
    anchors.right:        parent.right
    anchors.rightMargin:  5

    Component.onCompleted: {
      test_list.model = setting_data._model;
    }

    // 右侧测试参数列表
    ListView{
      id:           test_list
      anchors.fill: parent
      spacing:      0
      clip:         true

      // 测试参数项委托
      delegate: Item{
        width:  listview_container.width
        height: 50

        Text {
          text:                   left_str
          font.family:            "Ubuntu"
          font.pixelSize:         20
          color:                  left_color
          anchors.verticalCenter: parent.verticalCenter
          anchors.left:           parent.left
          anchors.leftMargin:     10
        }
        Text {
          text:                   right_str
          font.family:            "Ubuntu"
          font.pixelSize:         20
          color:                  right_color
          anchors.verticalCenter: parent.verticalCenter
          anchors.right:          parent.right
          anchors.rightMargin:    10
        }

        Rectangle {
            height:              2
            color:               darkforeground_color
            anchors.bottom:      parent.bottom
            anchors.left:        parent.left
            anchors.leftMargin:  5
            anchors.right:       parent.right
            anchors.rightMargin: 5
        }
      }
    }
  }
}
/// end of code ///
