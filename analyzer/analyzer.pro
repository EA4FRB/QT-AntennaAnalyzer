#-------------------------------------------------
#
# Project created by QtCreator 2015-02-22T00:22:07
#
#-------------------------------------------------

QT       += xml core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = analyzer
TEMPLATE = app

CONFIG += serialport

SOURCES += main.cpp\
        mainwindow.cpp \
    scandata.cpp \
    graphcanvas.cpp \
    graph.cpp \
    graphcursor.cpp \
    eventreceiver.cpp \
    config.cpp \
    settingsdlg.cpp \
    sark110/hid.cpp \
    sark110/hid_WINDOWS.cpp \
    deviceio.cpp \
    bargraph.cpp \
    sark110/sark_client.cpp

HEADERS  += mainwindow.h \
    scandata.h \
    graphcanvas.h \
    graph.h \
    graphcursor.h \
    eventreceiver.h \
    version.h \
    config.h \
    settingsdlg.h \
    sark110/sark_cmd_defs.h \
    sark110/hidapi.h \
    sark110/hid.h \
    deviceio.h \
    bargraph.h \
    sark110/sark_client.h

FORMS    += mainwindow.ui \
    settingsdlg.ui

RESOURCES += \
    analyzer.qrc

unix {
LIBS += -ludev
}
INCLUDEPATH += sark110
win32 {
INCLUDEPATH += "C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\shared";"C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\um";"C:\WinDDK\7600.16385.1\inc\ddk";"C:\WinDDK\7600.16385.1\inc\api"
LIBS += C:\WinDDK\7600.16385.1\lib\wxp\i386\hid.lib
LIBS += C:\WinDDK\7600.16385.1\lib\wxp\i386\hidparse.lib
LIBS += C:\WinDDK\7600.16385.1\lib\wxp\i386\hidclass.lib
LIBS += C:\WinDDK\7600.16385.1\lib\wxp\i386\setupapi.lib
}
