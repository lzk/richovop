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
    app/vop_device.cpp \
    app/deviceapp.cpp \
    copiessettingkeyboard.cpp \
    scalingsettingkeyboard.cpp \
    app/devicecontrol.cpp \
    app/devicemanager.cpp

HEADERS  += mainwindow.h \
    mainwidget.h \
    app/vop_protocol.h \
    app/vop_device.h \
    lib/usb.h \
    lib/NetDevice.h \
    app/deviceapp.h \
    copiessettingkeyboard.h \
    scalingsettingkeyboard.h \
    app/devicecontrol.h \
    app/devicemanager.h

FORMS    += mainwindow.ui \
    setting.ui \
    mainwidget.ui \
    copy.ui \
    about.ui \
    copiessettingkeyboard.ui \
    scalingsettingkeyboard.ui

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
   -ldl \
   -lcups -L/usr/lib/i386-linux-gnu/  \

#    /usr/lib/i386-linux-gnu/libcups.so.2 \

}
DEFINES += DEVICE_LIB_SUPPORT

