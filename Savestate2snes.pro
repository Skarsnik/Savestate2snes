#-------------------------------------------------
#
# Project created by QtCreator 2017-11-01T11:44:23
#
#-------------------------------------------------

QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Savestate2snes
TEMPLATE = app


SOURCES += main.cpp\
        savestate2snesw.cpp \
    handlestuff.cpp \
    usb2snes.cpp \
    usb2snesstatut.cpp

HEADERS  += savestate2snesw.h \
    handlestuff.h \
    usb2snes.h \
    usb2snesstatut.h

FORMS    += savestate2snesw.ui \
    usb2snesstatut.ui

RC_FILE = savestate2snes.rc
