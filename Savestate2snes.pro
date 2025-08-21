#-------------------------------------------------
#
# Project created by QtCreator 2017-11-01T11:44:23
#
#-------------------------------------------------

QT       += core gui websockets gamepad

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

GIT_TAG_VERSION=$$system(git describe --always --tags)
DEFINES += GIT_TAG_VERSION=\\\"$$GIT_TAG_VERSION\\\"

TARGET = Savestate2snes
TEMPLATE = app
CONFIG += no_batch

win32 {
    LIBS += -ldinput8 -ldxguid
    CONFIG += c++11
    SOURCES += Button-Mash/directinputsource.cpp
    HEADERS += Button-Mash/directinputsource.h
    SOURCES += Button-Mash/QGameController/src/gamecontroller/qgamecontroller.cpp
    SOURCES += Button-Mash/QGameController/src/gamecontroller/qgamecontroller_win.cpp
    QGC_PUBLIC_HEADERS += Button-Mash/QGameController/src/gamecontroller/qgamecontroller.h
    QGC_PRIVATE_HEADERS += Button-Mash/QGameController/src/gamecontroller/qgamecontroller_p.h
    HEADERS += $$QGC_PUBLIC_HEADERS $$QGC_PRIVATE_HEADERS
    message("Compiling for windows")
}


SOURCES += main.cpp\
        savestate2snesw.cpp \
    handlestuff.cpp \
    trainingconfigdialog.cpp \
    usb2snes.cpp \
    usb2snesstatut.cpp \
    firsttimedialog.cpp \
    shortcuteditdialog.cpp \
    handlestuffusb2snes.cpp \
    Button-Mash/inputdecoder.cpp \
    Button-Mash/inputprovider.cpp \
    Button-Mash/mapbuttondialog.cpp \
    Button-Mash/localcontroller.cpp \
    Button-Mash/localcontrollermanager.cpp \
    Button-Mash/qgamepadsource.cpp \
    snesclassicstuff/stuffclient/stuffclient.cpp \
    snesclassicstatut.cpp \
    handlestuffsnesclassic.cpp \
    consoleswitcher.cpp \
    savestatelistview.cpp \
    handlestuffnwaccess.cpp \
    nwaccessstatut.cpp \
    trainingtimer.cpp


HEADERS  += savestate2snesw.h \
    handlestuff.h \
    trainingconfigdialog.h \
    usb2snes.h \
    usb2snesstatut.h \
    firsttimedialog.h \
    shortcuteditdialog.h \
    handlestuffusb2snes.h \
    snesclassicstuff/stuffclient/stuffclient.h \
    Button-Mash/inputdecoder.h \
    Button-Mash/inputprovider.h \
    Button-Mash/mapbuttondialog.h \
    Button-Mash/localcontroller.h \
    Button-Mash/localcontrollermanager.h \
    Button-Mash/qgamepadsource.h \
    snesclassicstatut.h \
    handlestuffsnesclassic.h \
    consoleswitcher.h \
    savestatelistview.h \
    handlestuffnwaccess.h \
    nwaccessstatut.h \
    trainingtimer.h

FORMS    = savestate2snesw.ui \
    trainingconfigdialog.ui \
    usb2snesstatut.ui \
    firsttimedialog.ui \
    shortcuteditdialog.ui \
    snesclassicstatut.ui \
    consoleswitcher.ui \
    nwaccessstatut.ui \
    Button-Mash/mapbuttondialog.ui \
    trainingtimer.ui

include($$PWD/EmuNWAccess-qt/EmuNWAccess-qt.pri)
INCLUDEPATH += snesclassicstuff/desktopclient/InputDisplay/

RC_FILE = savestate2snes.rc

macx: {
        QMAKE_INFO_PLIST = Info.plist
        ICON = Icon126x126.icns
}



RESOURCES += \
    images.qrc

TRANSLATIONS = Translations\savestate2snes_fr.ts \
               Translations\savestate2snes_de.ts \
               Translations\savestate2snes_sv.ts \
               Translations\savestate2snes_nl.ts

DISTFILES += \
    memorypreset.json
