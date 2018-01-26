#-------------------------------------------------
#
# Project created by QtCreator 2017-11-01T11:44:23
#
#-------------------------------------------------

#include(qtftp/modules/qt_ftp.pri)

QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Savestate2snes
TEMPLATE = app


SOURCES += main.cpp\
        savestate2snesw.cpp \
    handlestuff.cpp \
    usb2snes.cpp \
    usb2snesstatut.cpp \
    firsttimedialog.cpp \
    shortcuteditdialog.cpp \
    handlestuffusb2snes.cpp \
    snesclassicstuff/desktopclient/miniftp.cpp \
    snesclassicstuff/desktopclient/telnetconnection.cpp \
    snesclassicstatut.cpp \
    handlestuffsnesclassic.cpp \
    qtftp/src/qftp/qftp.cpp \
    qtftp/src/qftp/qurlinfo.cpp

HEADERS  += savestate2snesw.h \
    handlestuff.h \
    usb2snes.h \
    usb2snesstatut.h \
    firsttimedialog.h \
    shortcuteditdialog.h \
    handlestuffusb2snes.h \
    snesclassicstuff/desktopclient/miniftp.h \
    snesclassicstuff/desktopclient/telnetconnection.h \
    snesclassicstatut.h \
    handlestuffsnesclassic.h \
    qtftp/src/qftp/qftp.h \
    qtftp/src/qftp/qurlinfo.h

FORMS    += savestate2snesw.ui \
    usb2snesstatut.ui \
    firsttimedialog.ui \
    shortcuteditdialog.ui \
    snesclassicstatut.ui

RC_FILE = savestate2snes.rc

RESOURCES += \
    images.qrc

TRANSLATIONS = Translations\savestate2snes_fr.ts \
               Translations\savestate2snes_de.ts \
               Translations\savestate2snes_sv.ts \
               Translations\savestate2snes_nl.ts
