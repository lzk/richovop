#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T16:13:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = vop
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mainwidget.cpp \
    app/vop_protocol.cpp \
    app/devicemanager.cpp \
    app/vop_device.cpp \
    app/deviceapp.cpp

HEADERS  += mainwindow.h \
    mainwidget.h \
    app/devicemanager.h \
    app/vop_protocol.h \
    app/vop_device.h \
    lib/usb.h \
    lib/NetDevice.h \
    app/deviceapp.h

FORMS    += mainwindow.ui \
    setting.ui \
    mainwidget.ui \
    copy.ui \
    about.ui

RESOURCES += \
    vop.qrc

TRANSLATIONS = translations/vop_zh.ts

INCLUDEPATH += cups

LIBS +=   -ldl

PLATFORM = $$system(uname -i)
contains(PLATFORM, x86_64){
LIBS += \
    /usr/lib/x86_64-linux-gnu/libcups.so.2
}else{
LIBS += \
    /usr/lib/i386-linux-gnu/libcups.so.2 \

  #  -lcups -L/usr/lib/i386-linux-gnu/  \

}
DEFINES += DEVICE_LIB_SUPPORT

