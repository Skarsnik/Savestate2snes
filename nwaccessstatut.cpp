#include "nwaccessstatut.h"
#include "ui_nwaccessstatut.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log_NWAccessstatut, "NWAccessStatut")

#define sDebug() qCDebug(log_NWAccessstatut)


NWAccessStatut::NWAccessStatut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NWAccessStatut)
{
    ui->setupUi(this);
    client = new EmuNWAccessClient(this);
    checkingInfo = false;
    checkingStatus = false;

    checkStatusTimer.setInterval(3000);
    connect(&checkStatusTimer, &QTimer::timeout, this, &NWAccessStatut::onTimerTimeout);
    connect(client, &EmuNWAccessClient::readyRead, this, [=] {
        auto reply = client->readReply();
        //sDebug() << reply.toMap();
        if (checkingStatus)
        {
            checkingStatus = false;
            if (reply["state"] == "no_game" || reply["game"].isEmpty())
            {
                ui->gameLabel->setText(tr("No game loaded"));
                emit unReadyForSaveState();
            } else {
                ui->gameLabel->setText(QString(tr("Running : %1").arg(reply["game"])));
                checkStatusTimer.stop();
                emit readyForSaveState();
            }
        }
        if (checkingInfo)
        {
            checkingStatus = true;
            checkingInfo = false;
            ui->emulatorLabel->setText(reply["name"] + " " + reply["version"]);
            client->cmdEmulationStatus();
        }
    });

    connect(client, &EmuNWAccessClient::connected, this, [=] {
        sDebug() << "Connected";
        client->cmdMyNameIs("Savestate2Snes status check");
    });
    connect(client, &EmuNWAccessClient::disconnected, this, [=] {
        sDebug() << "Disconnected";
        ui->gameLabel->setText(QString("No emulator running"));
        ui->emulatorLabel->setText(QString("No emulator running"));
        checkStatusTimer.start();
        emit unReadyForSaveState();
    });
}

void    NWAccessStatut::stop()
{
    client->disconnectFromHost();
    checkStatusTimer.stop();
}

QString NWAccessStatut::readyString()
{
    return tr("Emulator ready for Savestate");
}

QString NWAccessStatut::unreadyString()
{
    return tr("Emulator not ready for Savestate");
}

void NWAccessStatut::onTimerTimeout()
{
    sDebug() << client->isConnected();
    if (client->isConnected())
    {
        sDebug() << "Cmd info";
        client->cmdEmulatorInfo();
        checkingInfo = true;
    } else {
        client->connectToHost("127.0.0.1", 65400);
    }
}

void    NWAccessStatut::start()
{
    client->connectToHost("127.0.0.1", 65400);
    checkStatusTimer.start();
}



NWAccessStatut::~NWAccessStatut()
{
    delete ui;
}
