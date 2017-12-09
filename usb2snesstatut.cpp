#include "usb2snesstatut.h"
#include "ui_usb2snesstatut.h"

#define CHECK_ROMRUNNING_TICK 500

USB2SnesStatut::USB2SnesStatut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::USB2SnesStatut)
{
     ui->setupUi(this);
     connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
     timer.start(1000);
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
        ui->romPatchedLabel->setText(tr("ROM is patched for savestate"));
        emit readyForSaveState();
    }

}

//To see if any NMI hook patch is applied you could technically look at $002A90 to see if it's $60 (RTS) = no patch.

bool USB2SnesStatut::isPatchedRom()
{
    QByteArray data = usb2snes->getAddress(0x2A90, 1);
    if (data[0] != (char) 0x60)
        return true;
    return false;
}


void USB2SnesStatut::on_patchROMpushButton_clicked()
{
    timer.stop();
    if (usb2snes->patchROM(qApp->applicationDirPath() + "/Patches/savestate.ips"))
    {
        ui->patchROMpushButton->setEnabled(false);
        emit readyForSaveState();
    }
    timer.start(CHECK_ROMRUNNING_TICK);
}

void USB2SnesStatut::onTimerTick()
{
    emit readyForSaveState();
    timer.stop();
}

void USB2SnesStatut::onUsb2snesStateChanged()
{
    if (usb2snes->state() == USB2snes::Ready)
    {
        timer.start(CHECK_ROMRUNNING_TICK);
    }
}


USB2SnesStatut::~USB2SnesStatut()
{
    delete ui;
}

void USB2SnesStatut::setUsb2snes(USB2snes *usnes)
{
    usb2snes = usnes;
    connect(usb2snes, SIGNAL(stateChanged()), this, SLOT(onUsb2snesStateChanged()));
}
