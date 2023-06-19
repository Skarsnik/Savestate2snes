#include "nwaccessstatut.h"
#include "ui_nwaccessstatut.h"
#include <QLoggingCategory>
#include <Button-Mash/localcontrollermanager.h>
#include <Button-Mash/mapbuttondialog.h>

Q_LOGGING_CATEGORY(log_NWAccessstatut, "NWAccessStatut")

#define sDebug() qCDebug(log_NWAccessstatut)

const QString SETTING_CONTROLLERID = "NWAControllerID";
const QString SETTING_CONTROLLERMAPPING = "NWAControllerMapping";

NWAccessStatut::NWAccessStatut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NWAccessStatut)
{
    ui->setupUi(this);
    client = new EmuNWAccessClient(this);
    checkingInfo = false;
    checkingStatus = false;
    localController = nullptr;

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

void NWAccessStatut::setSettings(QSettings *set)
{
    settings = set;
}

void NWAccessStatut::setToShow()
{
    if (settings->contains(SETTING_CONTROLLERID))
    {
        sDebug() << "Creating localcontroller" << settings->value(SETTING_CONTROLLERID).toString();
        localController = LocalControllerManager::getManager()->createProvider(settings->value(SETTING_CONTROLLERID).toString());
        mapping = LocalControllerManager::getManager()->loadMapping(*settings, SETTING_CONTROLLERMAPPING);
        localController->setMapping(mapping);
        ui->controllerComboBox->addItem(localController->name());
        ui->controllerComboBox->setItemData(0, localController->id(), Qt::UserRole + 1);
        emit localControllerChanged();
    }
    listController();
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
        client->connectToHost("127.0.0.1", 0xBEEF);
    }
}

void NWAccessStatut::listController()
{
    controllersInfos = LocalControllerManager::getManager()->listController();
    quint8 cpt = 0;
    ui->controllerComboBox->clear();
    for (auto& info : controllersInfos)
    {
        ui->controllerComboBox->addItem(info.name);
        ui->controllerComboBox->setItemData(cpt, info.id, Qt::UserRole + 1);
        cpt++;
    }
}

void    NWAccessStatut::start()
{
    client->connectToHost("127.0.0.1", 0xBEEF);
    checkStatusTimer.start();
}



NWAccessStatut::~NWAccessStatut()
{
    delete ui;
}

void NWAccessStatut::on_refreshPushButton_clicked()
{
    listController();
}


void NWAccessStatut::on_mappingButton_clicked()
{
    MapButtonDialog diag;
    diag.setMapping(mapping);
    int idx = ui->controllerComboBox->currentIndex();
    diag.setDevice(controllersInfos.at(idx));
    if (diag.exec())
    {
        mapping = diag.mapping();
        if (localController == nullptr)
            localController = LocalControllerManager::getManager()->createProvider(ui->controllerComboBox->itemData(idx, Qt::UserRole + 1).toString());
        localController->setMapping(mapping);
        localController = LocalControllerManager::getManager()->createProvider(
          ui->controllerComboBox->itemData(ui->controllerComboBox->currentIndex(), Qt::UserRole + 1).toString());
        settings->setValue(SETTING_CONTROLLERID, ui->controllerComboBox->itemData(idx, Qt::UserRole + 1).toString());
        LocalControllerManager::getManager()->saveMapping(*settings, SETTING_CONTROLLERMAPPING, mapping);
        emit localControllerChanged();
    }
}

