#include "snesclassicstatut.h"
#include "ui_snesclassicstatut.h"

SNESClassicStatut::SNESClassicStatut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNESClassicStatut)
{
    ui->setupUi(this);
    miniFtp = new MiniFtp();
    //cmdCo = new TelnetConnection("localhost", 1023, "root", "clover");

    canoeRunning = false;
    ftpReady = false;

    connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));

    connect(this, SIGNAL(canoeStarted()), this, SLOT(onCanoeStarted()));
    connect(this, SIGNAL(canoeStopped()), this, SLOT(onCanoeStopped()));
    miniFtp->connect();
}

void SNESClassicStatut::onTimerTick()
{
    ftpReady = miniFtp->state() == MiniFtp::Connected;
    if (cmdCo->state() == TelnetConnection::Ready || cmdCo->state() == TelnetConnection::Connected)
    {
        QByteArray result = cmdCo->syncExecuteCommand("pidof canoe-shvc > /dev/null && echo 1 || echo 0");
        bool oldcr = canoeRunning;
        //qDebug() << result << result.trimmed();
        canoeRunning = (result.trimmed() == "1");
        qDebug() << canoeRunning;
        if (oldcr == false && canoeRunning == true)
            emit canoeStarted();
        if (oldcr == true && canoeRunning == false)
            emit canoeStopped();
        timer.setInterval(5000);
    } else {
        if (cmdCo->state() == TelnetConnection::Offline)
            cmdCo->conneect();
        timer.setInterval(5000);
    }
}


SNESClassicStatut::~SNESClassicStatut()
{
    timer.stop();
    cmdCo->close();
    delete ui;
}

SNESClassicStatut::setCommandCo(TelnetConnection *telco)
{
    cmdCo = telco;
    connect(cmdCo, SIGNAL(connectionError(TelnetConnection::ConnectionError)), this, SLOT(onCommandCoError(TelnetConnection::ConnectionError)));
    connect(cmdCo, SIGNAL(disconnected()), this, SLOT(onCommandCoDisconnected()));
    connect(cmdCo, SIGNAL(connected()), this, SLOT(onCommandCoConnected()));
    cmdCo->conneect();
    timer.start(500);
}

void SNESClassicStatut::onCanoeStarted()
{
    if (ftpReady)
    {
        QByteArray ba = cmdCo->syncExecuteCommand("ps | grep canoe | grep -v grep");
        QString result = ba.trimmed();
        QString canoeStr = result.mid(result.indexOf("canoe"));
        QStringList canoeArgs = canoeStr.split(" ");
        ui->romNameLabel->setText(canoeArgs.at(canoeArgs.indexOf("-rom") + 1));
        timer.stop();
        emit readyForSaveState();
    }
}

void SNESClassicStatut::onCanoeStopped()
{
    ui->romNameLabel->setText(tr("Canoe not running"));
    emit unReadyForSaveState();
}

void SNESClassicStatut::onCommandCoConnected()
{

}

void SNESClassicStatut::onCommandCoDisconnected()
{
    emit unReadyForSaveState();
}

void SNESClassicStatut::onCommandCoError(TelnetConnection::ConnectionError)
{
    emit unReadyForSaveState();
}
