/*
    This file is part of the SaveState2snes software
    Copyright (C) 2017  Sylvain "Skarsnik" Colinet <scolinet@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef USB2SNESSTATUT_H
#define USB2SNESSTATUT_H

#include "usb2snes.h"

#include <QWidget>

namespace Ui {
class USB2SnesStatut;
}

class USB2SnesStatut : public QWidget
{
    Q_OBJECT

public:
    explicit USB2SnesStatut(QWidget *parent = nullptr);
    ~USB2SnesStatut();
    void    setUsb2snes();
    void    refreshShortcuts();
    QString readyString() const;
    QString unreadyString() const;
    void    stop();

signals:
    void    readyForSaveState();
    void    unReadyForSaveState();

public slots:
    void on_patchROMpushButton_clicked();


private slots:
    void    onTimerTick();
    void    onUsb2snesStateChanged();
    void    onUsb2snesDisconnected();
    void    onRomStarted();
    void    buildStatusInfo();
    bool    validVersion();

    void on_pushButton_clicked();

    void on_statusPushButton_clicked();

private:
    Ui::USB2SnesStatut *ui;
    USB2snes*           usb2snes;

    QTimer              timer;
    bool                connectedOnce;
    bool                readyOnce;
    bool                isPatchedRom();
    bool                romRunning;
    bool                menuRunning;
    void                romPatched();
};

#endif // USB2SNESSTATUT_H
