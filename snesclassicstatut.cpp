#include "snesclassicstatut.h"
#include "ui_snesclassicstatut.h"

#include <QThread>

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
        if (canoeCo->state() == TelnetConnection::Offline)
            canoeCo->conneect();
        timer.setInterval(5000);
    }
}


SNESClassicStatut::~SNESClassicStatut()
{
    timer.stop();
    cmdCo->close();
    delete ui;
}

SNESClassicStatut::setCommandCo(TelnetConnection *telco, TelnetConnection *canoe)
{
    cmdCo = telco;
    canoeCo = canoe;
    connect(cmdCo, SIGNAL(connectionError(TelnetConnection::ConnectionError)), this, SLOT(onCommandCoError(TelnetConnection::ConnectionError)));
    connect(cmdCo, SIGNAL(disconnected()), this, SLOT(onCommandCoDisconnected()));
    connect(cmdCo, SIGNAL(connected()), this, SLOT(onCommandCoConnected()));
    cmdCo->conneect();
    canoeCo->conneect();
    timer.start(1000);
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
        ui->iniButton->setEnabled(true);
        //emit readyForSaveState();
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


#define CLOVERSAVESTATEPATH "/tmp/savestate2snes.svt"
#define CLOVERROLLBACKPATH "/tmp/rollback/"
#define SCREENSHOTPATH "/tmp/savestate2snes.png"

void SNESClassicStatut::on_iniButton_clicked()
{
    QByteArray ba = cmdCo->syncExecuteCommand("ps | grep canoe | grep -v grep");
    QString result = ba.trimmed();
    QString canoeStr = result.mid(result.indexOf("canoe-shvc"));
    //sDebug() << "Canoe Str : " << canoeStr;
    QStringList canoeRun = canoeStr.split(" ");

    if (canoeRun.indexOf("-rollback-output-dir") == -1)
    {
        emit readyForSaveState();
        return ;
    }
    QStringList optArgToremove;
    optArgToremove << "-rollback-snapshot-period" << "--load-time-path" << "-rollback-input-dir"  << "-rollback-output-dir" << "--save-time-path" << "--wait-transition-fd";
    optArgToremove << "--finish-transition-fd" << "--start-transition-fd" << "--rollback-ui" << "-rollback-snapshot-period" << "-rollback-mode";
    foreach( QString arg, optArgToremove)
    {
        int i = canoeRun.indexOf(arg);
        if (i != -1)
        {
            qDebug() << "Removing : " << arg;
            canoeRun.removeAt(i);
            canoeRun.removeAt(i);
        }
    }
    int i = canoeRun.indexOf("--enable-sram-file-hash");
    if (i != -1)
        canoeRun.removeAt(i);
    QString canoePid = cmdCo->syncExecuteCommand("pidof canoe-shvc").trimmed();
    cmdCo->syncExecuteCommand(QString("kill -%1 `pidof canoe-shvc`").arg(2));
    cmdCo->syncExecuteCommand("test -e /tmp/plop || (sleep 1 && kill -2 `pidof ReedPlayer-Clover` && touch /tmp/plop)");
    cmdCo->syncExecuteCommand("wait " + canoePid);
    canoeRun.append("--save-on-quit");
    canoeRun.append(CLOVERSAVESTATEPATH);
    int sshot = canoeRun.indexOf("--save-screenshot-on-quit");
    canoeRun.removeAt(sshot);
    canoeRun.removeAt(sshot);
    canoeRun << "--save-screenshot-on-quit" << SCREENSHOTPATH;
    QThread::sleep(2);
    canoeCo->executeCommand(canoeRun.join(" ") + " 2>/dev/null");
    emit readyForSaveState();
}
