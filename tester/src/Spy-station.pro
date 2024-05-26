QT += quick

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


TARGET = YQ_Tester

INCLUDEPATH += misc/

SOURCES += \
        main.cpp \
        misc/data_model.cpp \
        misc/main_data.cpp \
        misc/canbus.cpp \
        misc/uds_tester.cpp \
        page_data/page_device_data.cpp \
        page_data/page_general_setting_data.cpp \
        page_data/page_general_info_data.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator s code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
  main.h \
  misc/data_model.h \
  misc/main_data.h \
  misc/canbus.h \
  misc/uds_tester.h \
  page_data/page_device_data.h \
  page_data/page_general_setting_data.h \
  page_data/page_general_info_data.h

DISTFILES +=
