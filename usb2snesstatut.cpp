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



#include "usb2snesstatut.h"
#include "ui_usb2snesstatut.h"

#include <QToolTip>

#define CHECK_ROMRUNNING_TICK 500
#define STATUS_PIX_RED ":/status button red.png"
#define STATUS_PIX_ORANGE ":/status button yellow.png"
#define STATUS_PIX_GREEN ":/status button green.png"


USB2SnesStatut::USB2SnesStatut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::USB2SnesStatut)
{
     ui->setupUi(this);
     connectedOnce = false;
     readyOnce = false;
     connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));  
     timer.start(1000);
}

void USB2SnesStatut::setUsb2snes(USB2snes *usnes)
{
    usb2snes = usnes;
    connect(usb2snes, SIGNAL(stateChanged()), this, SLOT(onUsb2snesStateChanged()));
    connect(usb2snes, SIGNAL(disconnected()), this, SLOT(onUsb2snesDisconnected()));
}

void USB2SnesStatut::onRomStarted()
{
    QString text = usb2snes->getRomName();
    ui->romNameLabel->setText(text);
    if (!isPatchedRom())
    {
        ui->romPatchedLabel->setText(tr("ROM is not patched for savestate"));
        ui->patchROMpushButton->setEnabled(true);
    } else {
        romPatched();
    }

}


//; 	A		B	  Y      X        L      R      	>		<		v		^	Start	  select
//DW #$0080, #$8000, #$4000, #$0040, #$0020, #$0010, #$0100, #$0200, #$0400, #$0800, #$1000, #$2000,

QString snesjoy2string(QByteArray input)
{
    QMap<quint16, QString> maskToButton;
    maskToButton[0x1000] = "start";
    maskToButton[0x2000] = "select";
    maskToButton[0x0080] = "A";
    maskToButton[0x8000] = "B";
    maskToButton[0x4000] = "Y";
    maskToButton[0x0040] = "X";
    maskToButton[0x0020] = "L";
    maskToButton[0x0010] = "R";
    maskToButton[0x0100] = "right";
    maskToButton[0x0200] = "left";
    maskToButton[0x0400] = "down";
    maskToButton[0x0800] = "up";
    quint16 nInput = (input.at(1) << 8) + input.at(0);
    //qDebug() << input << QString::number(nInput, 16);
    QStringList inputs;
    foreach(quint16 mask, maskToButton.keys())
    {
        if ((mask & nInput) == mask)
            inputs << maskToButton[mask];
    }
    return inputs.join("+");
}

void    USB2SnesStatut::romPatched()
{
    ui->patchROMpushButton->setEnabled(false);
    ui->romPatchedLabel->setText(tr("ROM is patched for savestate"));
    ui->statusPushButton->setIcon(QIcon(STATUS_PIX_GREEN));
    emit readyForSaveState();
    // save then load
    QByteArray saveButton = usb2snes->getAddress(0xFC2002, 2);
    QByteArray loadButton = usb2snes->getAddress(0xFC2004, 2);
    ui->shortcutLabel->setText(QString(tr("Shortcuts: - Save: %1 - Load: %2")).arg(snesjoy2string(saveButton)).arg(snesjoy2string(loadButton)));
    ui->shortcutLabel->setEnabled(true);
}

void USB2SnesStatut::on_patchROMpushButton_clicked()
{
    timer.stop();
    if (usb2snes->patchROM(qApp->applicationDirPath() + "/Patches/savestate.ips"))
        romPatched();
    timer.start(CHECK_ROMRUNNING_TICK);
}

void USB2SnesStatut::buildStatusInfo()
{
    QString statusString;
    if (!connectedOnce)
    {
        statusString = tr("Can't connect to USB2SNES application, probably not running.");
        goto setStatusToolTips;
    }
    if (!readyOnce)
    {
        statusString = tr("USB2SNES connection not ready\n");
        if (usb2snes->deviceList().isEmpty())
            statusString.append(tr("No sd2nes devices found."));
        goto setStatusToolTips;
    }
    statusString = QString(tr("SD2SNES On : %1\n")).arg(usb2snes->deviceList().at(0));
    statusString += QString(tr("Firmware version is %1 and USB2SNES app version : %2 : %3\n")).arg(usb2snes->firmwareVersion()).arg(usb2snes->serverVersion()).arg("OK");
    setStatusToolTips:
        qDebug() << statusString;
        ui->statusPushButton->setToolTip(statusString);
}

bool USB2SnesStatut::validVersion()
{
    if (usb2snes->firmwareVersion().right(1).toInt() >= 5 &&
        !usb2snes->serverVersion().isEmpty())
        return true;
    return false;
}

//To see if any NMI hook patch is applied you could technically look at $002A90 to see if it's $60 (RTS) = no patch.

bool USB2SnesStatut::isPatchedRom()
{
    QByteArray data = usb2snes->getAddress(0x2A90, 1, USB2snes::CMD);
    if (data[0] != (char) 0x60)
        return true;
    return false;
}

void USB2SnesStatut::onTimerTick()
{
    //emit readyForSaveState();
    timer.stop();
}

void USB2SnesStatut::onUsb2snesStateChanged()
{
    if (usb2snes->state() == USB2snes::Ready)
    {
        ui->statusPushButton->setIcon(QIcon(STATUS_PIX_ORANGE));;
        timer.start(CHECK_ROMRUNNING_TICK);
        readyOnce = true;
    }
    if (usb2snes->state() == USB2snes::Connected)
        connectedOnce = true;
    buildStatusInfo();
}

void USB2SnesStatut::onUsb2snesDisconnected()
{
    ui->statusPushButton->setIcon(QIcon(STATUS_PIX_RED));
    emit unReadyForSaveState();
}


USB2SnesStatut::~USB2SnesStatut()
{
    delete ui;
}



void USB2SnesStatut::on_pushButton_clicked()
{
    onRomStarted();
    //emit readyForSaveState();
}

void USB2SnesStatut::on_statusPushButton_clicked()
{
    QToolTip::showText(ui->statusPushButton->mapToGlobal(QPoint(0,0)), ui->statusPushButton->toolTip(), ui->statusPushButton, QRect(), 5000);
}
