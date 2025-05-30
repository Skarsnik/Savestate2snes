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

#include <QMessageBox>
#include <QToolTip>

Q_LOGGING_CATEGORY(log_usb2snesstatus, "USB2SNES Status")

#define sDebug() qCDebug(log_usb2snesstatus())

#define CHECK_ROMRUNNING_TICK 1000
#define STATUS_PIX_RED ":/status button red.png"
#define STATUS_PIX_ORANGE ":/status button yellow.png"
#define STATUS_PIX_GREEN ":/status button green.png"

extern bool dontLogNext;


USB2SnesStatut::USB2SnesStatut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::USB2SnesStatut)
{
     ui->setupUi(this);
     connectedOnce = false;
     readyOnce = false;
     connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));  
     //timer.start(1000);
     menuRunning = false;
     romRunning = false;
     usb2snes = new USB2snes("Savestate2Snes Status checker");
     ui->romPatchedLabel->hide();
}

void USB2SnesStatut::setUsb2snes()
{
    usb2snes->connect();
    connect(usb2snes, &USB2snes::stateChanged, this, &USB2SnesStatut::onUsb2snesStateChanged);
    connect(usb2snes, &USB2snes::disconnected, this, &USB2SnesStatut::onUsb2snesDisconnected);
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
    quint16 nInput = ((quint16) ((static_cast<uchar>(input.at(1)) << 8)) | ((quint16) static_cast<uchar>(input.at(0)) & 0x00FF));
    QStringList inputs;
    foreach(quint16 mask, maskToButton.keys())
    {
        if ((mask & nInput) == mask)
            inputs << maskToButton[mask];
    }
    return inputs.join("+");
}


void USB2SnesStatut::refreshShortcuts()
{
    unsigned int offset = 0xFC2000;
    if (usb2snes->firmwareVersion() >= QVersionNumber(11))
        offset = 0xFE1000;
    QByteArray saveButton = usb2snes->getAddress(offset + 2, 2);
    QByteArray loadButton = usb2snes->getAddress(offset + 4, 2);
    sDebug() << "Refreshing shortcuts : " << saveButton.toHex() << loadButton.toHex() << snesjoy2string(saveButton) << snesjoy2string(loadButton);
    ui->shortcutLabel->setText(QString(tr("Shortcuts: - Save: %1 - Load: %2")).arg(snesjoy2string(saveButton)).arg(snesjoy2string(loadButton)));
    ui->shortcutLabel->setEnabled(true);
}

QString USB2SnesStatut::readyString() const
{
    return tr("USB2Snes ready for savestate");
}

QString USB2SnesStatut::unreadyString() const
{
    return tr("USB2Snes not ready for savestate");
}

void USB2SnesStatut::stop()
{
    timer.stop();
}

void USB2SnesStatut::onRomStarted()
{
    sDebug() << "Rom started";
    QString text = usb2snes->infos().at(2);
    ui->romNameLabel->setText(text);
    /*if (usb2snes->firmwareVersion().majorVersion() == 8 || usb2snes->firmwareVersion().majorVersion() == 9)
    {
        ui->romPatchedLabel->setText(tr("Running USB2Snes V8/V9, I can't know if patched ROM"));
        ui->patchROMpushButton->setEnabled(true);
        return;
    }*/
    if (usb2snes->firmwareVersion() >= QVersionNumber(11))
    {
        QByteArray configData = usb2snes->getFile("/sd2snes/config.yml");
        sDebug() << "Got config file";
        if (configData.contains("EnableIngameSavestate: 1"))
        {
            romPatched();
        } else {
            ui->romPatchedLabel->setText(tr("Build in Savestate feature is not enabled"));
        }
        return ;
    }
    if (!isPatchedRom())
    {
        ui->romPatchedLabel->setText(tr("ROM is not patched for savestate"));
        ui->patchROMpushButton->setEnabled(true);
    } else {
        romPatched();
    }

}

void    USB2SnesStatut::romPatched()
{
    sDebug() << "Rom is patched";
    ui->patchROMpushButton->setEnabled(false);
    if (usb2snes->firmwareVersion() >= QVersionNumber(11))
    {
        ui->romPatchedLabel->setText(tr("Build in Savestate feature is enabled"));
    } else {
        ui->romPatchedLabel->setText(tr("ROM is patched for savestate"));
    }
    ui->statusPushButton->setIcon(QIcon(STATUS_PIX_GREEN));
    emit readyForSaveState();
    refreshShortcuts();
}

void USB2SnesStatut::on_patchROMpushButton_clicked()
{
    timer.stop();
    QString patchFile;
    if (usb2snes->firmwareVersion() < QVersionNumber(8))
        patchFile = "savestatev7.ips";
    else
        patchFile = "savestatev8.ips";
    if (usb2snes->patchROM(qApp->applicationDirPath() + "/Patches/" + patchFile))
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
        statusString = tr("USB2SNES connection not ready") + "\n";
        if (usb2snes->deviceList().isEmpty())
            statusString.append(tr("No sd2nes devices found."));
        goto setStatusToolTips;
    }
    if (menuRunning)
    {
        statusString = tr("SD2SNES is on menu, not a rom") + "\n";
    }
    statusString += QString(tr("SD2SNES On : %1")).arg(usb2snes->deviceList().at(0)) + "\n";
    statusString += QString(tr("Firmware version is %1 and USB2SNES app version : %2 : %3")).arg(usb2snes->firmwareString()).arg(usb2snes->serverVersionString()).arg(validVersion() ? "OK" : "NOK");
    statusString += "\n";
    setStatusToolTips:
        sDebug() << statusString;
        ui->statusPushButton->setToolTip(statusString);
}

bool USB2SnesStatut::validVersion()
{
    if (usb2snes->firmwareVersion() >= QVersionNumber(6) && (!usb2snes->legacyConnection() || usb2snes->serverVersion() >= QVersionNumber(6)))
        return true;
    return false;
}

//To see if any NMI hook patch is applied you could technically look at $002A90 to see if it's $60 (RTS) = no patch.

bool USB2SnesStatut::isPatchedRom()
{
    sDebug() << "Checking for patched rom";
    if (usb2snes->firmwareVersion() > QVersionNumber(7))
    {
        QByteArray data = usb2snes->getAddress(0xFC0000, 4, USB2snes::CMD);
        return data != QByteArray::fromHex("00000000");
    }
    QByteArray data = usb2snes->getAddress(0x2A90, 1, USB2snes::CMD);
    if (data[0] != (char) 0x60)
        return true;
    return false;
}

void USB2SnesStatut::onTimerTick()
{
    dontLogNext = true;
    // This flood the log too much
    QStringList infos = usb2snes->infos();
    dontLogNext = false;
    infos[2].truncate(infos.at(2).indexOf(QChar::Null));
    sDebug() << "tick infos : " << infos;
    if (infos.isEmpty())
        return ;
    bool isMenuRom = infos.at(2) == "/sd2snes/menu.bin" || infos.at(2) == "/sd2snes/m3nu.bin";
    if (isMenuRom == false && romRunning == false)
    {
        romRunning = true;
        menuRunning = false;
        onRomStarted();
        timer.setInterval(CHECK_ROMRUNNING_TICK * 2);
    }
    if (isMenuRom == true && menuRunning == false)
    {
        ui->romPatchedLabel->setText(tr("SD2SNES on Menu"));
        romRunning = false;
        menuRunning = true;
        buildStatusInfo();
        timer.setInterval(CHECK_ROMRUNNING_TICK);
        emit unReadyForSaveState();
    }
}

void USB2SnesStatut::onUsb2snesStateChanged()
{
    if (usb2snes->state() == USB2snes::None)
        usb2snes->connect();
    if (usb2snes->state() == USB2snes::Ready)
    {
        if (validVersion())
        {
            ui->statusPushButton->setIcon(QIcon(STATUS_PIX_ORANGE));
            usb2snes->setAppName("Savestate2snes");
            timer.start(CHECK_ROMRUNNING_TICK);
            readyOnce = true;
            ui->romPatchedLabel->show();
        } else {
            QMessageBox msgBox(this);
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setText(QString(tr("Your usb2snes client version (%1) and/or "
                                      "usb2snes firmware version (%2) are not enought to run Savestate2snes<br/>"
                                      "You need at least version 6 for both.<br/> You can get it at  <a href=\"https://github.com/RedGuyyyy/sd2snes/releases\"><span style=\" text-decoration: underline; color:#0000ff;\">USB2snes last release</span></a>"
                                      )).arg(usb2snes->serverVersion().toString()).arg(usb2snes->firmwareVersion().toString()));
            msgBox.setWindowTitle(tr("Version error"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
    }
    if (usb2snes->state() == USB2snes::Connected)
        connectedOnce = true;
    buildStatusInfo();
}

void USB2SnesStatut::onUsb2snesDisconnected()
{
    ui->statusPushButton->setIcon(QIcon(STATUS_PIX_RED));

    timer.stop();
    emit unReadyForSaveState();
}


USB2SnesStatut::~USB2SnesStatut()
{
    timer.stop();
    delete ui;
}



void USB2SnesStatut::on_pushButton_clicked()
{
    onRomStarted();
    //emit readyForSaveState();
}

void USB2SnesStatut::on_statusPushButton_clicked()
{
    if (usb2snes->state() == USB2snes::None)
    {
        usb2snes->connect();
    }
    QToolTip::showText(ui->statusPushButton->mapToGlobal(QPoint(0,0)), ui->statusPushButton->toolTip(), ui->statusPushButton, QRect(), 5000);
}
