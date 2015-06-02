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
    app/devicemanager.cpp \
    app/vop_device.cpp

HEADERS  += mainwindow.h \
    mainwidget.h \
    app/devicemanager.h \
    app/vop_protocol.h \
    app/vop_device.h

FORMS    += mainwindow.ui \
    setting.ui \
    mainwidget.ui \
    copy.ui \
    about.ui \
    pagewifisetup.ui \
    pagemodifypassword.ui \
    pagesearchwifi.ui \
    pagemanualsetup.ui

RESOURCES += \
    vop.qrc

TRANSLATIONS = translations/vop_zh.ts

INCLUDEPATH += /opt/cups-2.0.2/include

LIBS += -lcups -L/opt/cups-2.0.2/lib64
