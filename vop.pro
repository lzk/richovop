#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T16:13:33
#
#-------------------------------------------------

QT       += core gui network
#QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = "RICOH Printer"
TEMPLATE = app
#CONFIG += debug

SOURCES += main.cpp \
    mainwindow.cpp \
    mainwidget.cpp \
    app/vop_protocol.cpp \
    app/deviceapp.cpp \
    copiessettingkeyboard.cpp \
    scalingsettingkeyboard.cpp \
    app/devicecontrol.cpp \
    app/devicemanager.cpp \
    app/log.cpp \
    dialoglogin.cpp \
    tabsetting.cpp \
    tabcopy.cpp \
    tababout.cpp \
    app/usbdevice.cpp \
    app/netdevice.cpp \
    app/device.cpp \
    app/linux_api.cpp \
    app/devicedata.cpp \
    logo_icon.cpp \
    ricohmessagebox.cpp

HEADERS  += mainwindow.h \
    mainwidget.h \
    app/vop_protocol.h \
    app/deviceapp.h \
    copiessettingkeyboard.h \
    scalingsettingkeyboard.h \
    app/devicecontrol.h \
    app/devicemanager.h \
    app/log.h \
    version.h \
    dialoglogin.h \
    tabsetting.h \
    tabcopy.h \
    tababout.h \
    app/usbdevice.h \
    app/netdevice.h \
    app/device.h \
    app/linux_api.h \
    app/devicedata.h \
    logo_icon.h \
    ricohmessagebox.h

FORMS    += mainwindow.ui \
    mainwidget.ui \
    copiessettingkeyboard.ui \
    scalingsettingkeyboard.ui \
    dialoglogin.ui \
    tabsetting.ui \
    tabcopy.ui \
    tababout.ui \
    logo_icon.ui

RESOURCES += \
    vop.qrc

TRANSLATIONS = translations/vop.en.ts translations/vop.zh_CN.ts translations/vop.ru.ts

#English	SC	Japanese	French	Italian	German
#Spanish	Danish	Dutch	Norwegian	Swedish
#Russian	Turkish	Iberian-Portuguese	Brazilian-Portuguese Polish
#Czech	Hungarian	TC	Greek Finnish	Catalan

#TRANSLATIONS = translations/vop.en.ts translations/vop.zh_CN.ts translations/vop.ja.ts translations/vop.fr.ts translations/vop.it.ts translations/vop.de.ts \
#                                translations/vop.es.ts translations/vop.da.ts translations/vop.nl.ts translations/vop.nb.ts translations/vop.sv.ts\
#                                translations/vop.ru.ts translations/vop.tr.ts translations/vop.pt.ts translations/vop.pt_br.ts translations/vop.pl.ts \
#                                translations/vop.cs.ts translations/vop.hu.ts translations/vop.zh_TW.ts translations/vop.el.ts translations/vop.fi.ts translations/vop.ca.ts

LIBS += -Wl,-rpath,"/opt/RICOH/app/RICOH SP 150SU_SP 150" -ldl \
     -Wl,--wrap=memcpy \
# -Xlinker --hash-style=sysv -Xlinker -fno-stack-protector

#st = $$system("cat /etc/issue|awk '{print tolower($1)}' ")
#contains(st ,ubuntu) {
#    LIBS += `cups-config --libs`
#}else{
#    lb=$$system(getconf LONG_BIT)
#    contains(lb ,64){
#        LIBS += /usr/lib64/libcups.so.2
#    }else{
#        LIBS += /usr/lib/libcups.so.2
#    }
#    QTPLUGIN += qjpeg qtiff qmng qgif
#    DEFINES += STATIC_BUILD
#}

equals(QT_MAJOR_VERSION,4){
    QTPLUGIN += qjpeg qtiff qmng qgif qico
    DEFINES += STATIC_BUILD
}
#DEFINES += FUTURE_SUPPORT

#RC_FILE = images/RICOH_logo_171x055.gif
