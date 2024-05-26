import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2

//import qml.custom.Main_data 1.0

Window {
  id:      root_page
  visible: true
  height:  400;//Screen.desktopAvailableHeight
  width:   800;//Screen.desktopAvailableWidth
  color:   "transparent"


  property color heightlight_color     : "#b31312"
  property color lightforeground_color : "#ea906c"
  property color darkforeground_color  : "#2b2a4c"
  property color background_color      : "#eee2de"

  property color lightgray_color:      "#CCCCCC"
  property color lightblue_color:     "#6699CC" //"#DBE4CC"
  //property color background_color:      "#003366"
  // list width
  property int width_of_list:300// width/4
  property int width_of_page:500// width - width/4 -10
  //
  property int menu_index:0
  property int running:0

  //
  property var  init_page : Page_gerneral_info{}
  property var  device_page : Page_device_info{}
  property var  setting_page : Page_gerneral_setting{}

  Item{
    id:     root_item
    width:  root_page.width
    height: root_page.height

    Component.onCompleted: {
      test_list.model = Main_Data._model;

      ctrl_btn_txt.text    = (Main_Data._run === 1)?"点击暂停":"点击启动"
      ctrl_btn.border.color= (Main_Data._run === 1)?"#F33C70":"#8BF13C"
    }
    Connections {
      target: Main_Data
      onRun_tick:{
        if ( _flag === 0 ) 
          ctrl_btn.color = "#8BF13C"
        else 
          ctrl_btn.color = "transparent"

        acc_status.border.color = (_acc===1)?"#F33C70":"lightgray"
        bat_status.border.color = (_bat===1)?"#F33C70":"lightgray"
      }
    }

    // 顶部消息栏,包含控制按钮,启动时间和版本号
    Rectangle{
      id:                  info_container
      height:              50
      anchors.top:         parent.top
      anchors.topMargin:   5
      anchors.left:        parent.left
      anchors.leftMargin:  15
      anchors.right:       parent.right
      anchors.rightMargin: 15
      color:               background_color
      radius:              7

      Rectangle{
        id:                   ctrl_btn 
        width:                280 
        anchors.top:          parent.top
        anchors.topMargin:    5
        anchors.bottom:       parent.bottom
        anchors.bottomMargin: 5
        anchors.left:         parent.left
        anchors.leftMargin:   10
        color:                "transparent"
        border.color:         lightforeground_color
        border.width:         2
        radius:               7

        Text {
          id:                  ctrl_btn_txt
          text:                "启动"
          font.pointSize:      15
          color:               darkforeground_color
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment:   Text.AlignVCenter
          anchors.verticalCenter: parent.verticalCenter
          anchors.horizontalCenter: parent.horizontalCenter
        }

        MouseArea {
         anchors.fill: parent
         onPressed: {
             parent.color = lightforeground_color
         }
         onReleased: {
             parent.color = "transparent"
         }
         onClicked: {
           Main_Data.run_pause_clicked();
           ctrl_btn_txt.text    = (Main_Data._run === 1)?"点击暂停":"点击运行"
           //ctrl_btn.border.color= (Main_Data._run === 1)?"#F33C70":"#8BF13C"
           if ( Main_Data._run === 1) {
             all_record_btn.border.color = darkforeground_color
             record_btn.border.color     = darkforeground_color
             reset_btn.border.color      = darkforeground_color
           } else {
             all_record_btn.border.color = "gray"
             record_btn.border.color     = "gray"
             reset_btn.border.color      = "gray"
           }
         }
        }
      }

      RowLayout{
        id: layout
        spacing: 10

        anchors.top:          parent.top
        anchors.topMargin:    5
        anchors.bottom:       parent.bottom
        anchors.bottomMargin: 5
        anchors.right: parent.right
        anchors.rightMargin: 5

        Rectangle{
          id:                   turnon_btn
          Layout.maximumHeight: 100
          Layout.minimumHeight: 30
          Layout.preferredHeight: parent.height
          Layout.maximumWidth: 300
          Layout.minimumWidth: 50
          Layout.preferredWidth: 100
          color:                "transparent"
          border.color:         darkforeground_color
          border.width:         1
          radius:               7

          Text {
            text:                "手动全开"
            font.pointSize:      15
            color:               darkforeground_color
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
          }

          MouseArea {
            anchors.fill: parent
            onPressed: {
              parent.color = lightforeground_color
            }
            onReleased: {
              parent.color = "transparent"
            }
            onClicked: {
              Main_Data.manual_ctrl_click(1);
              ctrl_btn_txt.text ="点击恢复运行"
            }
          }
        }

        Rectangle{
          id:                   turnoff_btn
          Layout.maximumHeight: 100
          Layout.minimumHeight: 30
          Layout.preferredHeight: parent.height
          Layout.maximumWidth: 300
          Layout.minimumWidth: 50
          Layout.preferredWidth: 100
          color:                "transparent"
          border.color:         darkforeground_color
          border.width:         1
          radius:               7

          Text {
            text:                "手动全关"
            font.pointSize:      15
            color:               darkforeground_color
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
          }

          MouseArea {
            anchors.fill: parent
            onPressed: {
              parent.color = lightforeground_color
            }
            onReleased: {
              parent.color = "transparent"
            }
            onClicked: {
              Main_Data.manual_ctrl_click(0);
              ctrl_btn_txt.text ="点击恢复运行"
            }
          }
        }


        Rectangle{
          id:                   all_record_btn
          //Layout.preferredHeight: 30
          Layout.maximumHeight: 100
          Layout.minimumHeight: 30
          Layout.preferredHeight: parent.height

          Layout.fillWidth: true
          Layout.minimumWidth: 50
          Layout.preferredWidth: 100
          Layout.maximumWidth: 300

          color:                "transparent"
          //border.color:         (Main_Data._run === 1)?"#8BF13C":"black"
          border.color:         darkforeground_color
          border.width:         1
          radius:               7

          Text {
            text:                "全记录"
            font.pointSize:      15
            color:               darkforeground_color
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
          }

          MouseArea {
            anchors.fill: parent
            onPressed: {
              parent.color = lightforeground_color
            }
            onReleased: {
              parent.color = "transparent"
            }
            onClicked: {
              Main_Data.dev_record_clicked(menu_index-1);
            }
          }
        }

        Rectangle{
          id:                   record_btn
          Layout.maximumHeight: 100
          Layout.minimumHeight: 30
          Layout.preferredHeight: parent.height

          Layout.fillWidth: true
          Layout.minimumWidth: 50
          Layout.preferredWidth: 100
          Layout.maximumWidth: 300

          color:                "transparent"
          //border.color:         "#8BF13C"
          border.color:         darkforeground_color
          border.width:         1
          radius:               7

          Text {
            text:                "单记录"
            font.pointSize:      15
            color:               darkforeground_color
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
          }

          MouseArea {
            anchors.fill: parent
            onPressed: {
              parent.color = lightforeground_color
            }
            onReleased: {
              parent.color = "transparent"
            }
            onClicked: {
              Main_Data.dev_record_clicked(menu_index-1);
            }
          }
        }

        Rectangle{
          id:                   reset_btn
          Layout.maximumHeight: 100
          Layout.minimumHeight: 30
          Layout.preferredHeight: parent.height

          Layout.fillWidth: true
          Layout.minimumWidth: 50
          Layout.preferredWidth: 100
          Layout.maximumWidth: 300
          color:                "transparent"
          //border.color:         "#8BF13C"
          border.color:         darkforeground_color
          border.width:         1
          radius:               7

          Text {
            text:                "回零"
            font.pointSize:      15
            color:               darkforeground_color
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
          }

          MouseArea {
            anchors.fill: parent
            onPressed: {
              parent.color = lightforeground_color
            }
            onReleased: {
              parent.color = "transparent"
            }
            onClicked: {
              Main_Data.clean_clicked();
            }
          }
        }
        /// 按钮完
        Rectangle{
          id:                   acc_status 
          width:                30 
          height:               30
          color:                "transparent"
          border.color:         lightforeground_color
          border.width:         4
          radius:               15 

          Text {
            text:                "A"
            font.pointSize:      10 
            color:               darkforeground_color
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
          }
        }

        Rectangle{
          id:                   bat_status 
          width:                30 
          height:               30
          color:                "transparent"
          border.color:         lightforeground_color
          border.width:         4
          radius:               15 

          Text {
            text:                "B"
            font.pointSize:      10 
            color:               darkforeground_color
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:   Text.AlignVCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
          }
        }
      }
    }

    // 左侧列表,测试统计和设备名等
    Rectangle{
      id:                   listview_container
      width:                root_page.width_of_list
      anchors.top:          info_container.bottom
      anchors.topMargin:    15
      anchors.bottom:       parent.bottom
      anchors.bottomMargin: 5
      anchors.left:         parent.left
      anchors.leftMargin:   15
      color:                background_color
      radius:               7

      ListView{
        id:           test_list
        anchors.fill: parent
        spacing:      0
        clip:         true

        delegate: Item{
          width:  parent.width
          height: 60

          Text {
            id : left_string
            text:                   left_str
            font.family:            "Ubuntu"
            font.pixelSize:         25
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
            //anchors.verticalCenter: parent.verticalCenter
            anchors.bottom        : left_string.bottom
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

          // 存在问题：
          // 如果委托被触摸，则模型内容更新后，不会更新到视图
          // 如果没有交互，则模型内容的更新可以正常更新到视图
          MouseArea {
            anchors.fill: parent
            onClicked: {
              Main_Data._menu_index = index;
              menu_index = index;
              if ( index > 1 ) {
                root_stack.push({item:device_page})
              } else if ( index === 1 ) {
                root_stack.push({item:setting_page})
              } else {
                root_stack.pop(init_page);
              }
            }
          }
        }
      }
    }

    // 右侧页面堆栈,包含:统计信息页面和设备参数页面
    Item{
      id:                   stackview_container
      anchors.top:          info_container.bottom
      anchors.topMargin:    15
      anchors.bottom:       parent.bottom
      anchors.bottomMargin: 5
      anchors.left:         listview_container.right
      anchors.leftMargin:   10
      anchors.right:        parent.right
      anchors.rightMargin:  15
      //radius:               15

      StackView {
          id:           root_stack
          anchors.fill: parent
          focus:        true
          visible:      true
          delegate:     StackViewDelegate {}
          initialItem:  init_page
      }
    }
  }
}

