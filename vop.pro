#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T16:13:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vop
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mainwidget.cpp \
    app/vop_protocol.cpp \
    app/vop_device.cpp \
    app/device.cpp \
    newwidget.cpp

HEADERS  += mainwindow.h \
    mainwidget.h \
    app/vop_protocol.h \
    app/device.h \
    newwidget.h

FORMS    += mainwindow.ui \
    setting.ui \
    mainwidget.ui \
    copy.ui \
    about.ui \
    newwidget.ui

RESOURCES += \
    vop.qrc

TRANSLATIONS = translations/vop_zh.ts

INCLUDEPATH += /opt/cups-2.0.2/include

LIBS += -lcups -L/opt/cups-2.0.2/lib64
