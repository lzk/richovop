#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T16:13:33
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = VOP
TEMPLATE = app


SOURCES += main.cpp \
        mainwindow.cpp \
    mainwidget.cpp \
    app/vop_protocol.cpp \
    app/vop_device.cpp \
    app/deviceapp.cpp \
    copiessettingkeyboard.cpp \
    scalingsettingkeyboard.cpp \
    app/devicecontrol.cpp \
    app/devicemanager.cpp \
    app/log.cpp

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
    app/devicemanager.h \
    app/log.h

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

LIBS += -Wl,-rpath,"/opt/RICOH/app/Ricoh SP 150SU_SP 150" -ldl `cups-config --libs`
#LIBS += -Wl,-rpath,"/opt/RICOH/app/Ricoh SP 150SU_SP 150"  -ldl /usr/lib64/libcups.so.2

