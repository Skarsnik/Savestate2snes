#include "snesclassicstatut.h"
#include "ui_snesclassicstatut.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QThread>


Q_LOGGING_CATEGORY(log_SNESClassicstatut, "SNESClassicStatut")

#define sDebug() qCDebug(log_SNESClassicstatut)

SNESClassicStatut::SNESClassicStatut(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNESClassicStatut)
{
    ui->setupUi(this);
    canoeRunning = false;
    m_settings = new QSettings("skarsnik.nyo.fr", "SaveState2SNES");
    connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    connect(this, SIGNAL(canoeStarted()), this, SLOT(onCanoeStarted()));
    connect(this, SIGNAL(canoeStopped()), this, SLOT(onCanoeStopped()));
    ui->coStatusLabel->setPixmap(QPixmap(":/snesclassic status button red.png"));
    ui->shortcutCheckBox->setChecked(m_settings->value("SNESClassicShortcuts").toBool());
}

void SNESClassicStatut::onTimerTick()
{
    sDebug() << "Timer tick";
    //timer.stop();
    if (!checkForReady())
    {
        sDebug() << "Not ready";
        if (!controlCo->isConnected())
            controlCo->connect();
        timer.start(1000);
    }
    else
        timer.stop();

}

bool SNESClassicStatut::checkForReady()
{
    if (controlCo->isConnected())
    {
        sDebug() << "Checking for canoe running";
        QByteArray result = controlCo->waitForCommand("pidof canoe-shvc > /dev/null && echo 1 || echo 0");
        bool oldcr = canoeRunning;
        sDebug() << "Is canoerunning?" << canoeRunning;
        //sDebug() << result << result.trimmed();
        canoeRunning = (result.trimmed() == "1");
        qDebug() << canoeRunning;
        if (oldcr == false && canoeRunning == true)
        {
            emit canoeStarted();
            return true;
        }
        if (oldcr == true && canoeRunning == false)
            emit canoeStopped();
    }
    return false;
}



SNESClassicStatut::~SNESClassicStatut()
{
    timer.stop();
    delete ui;
}

void SNESClassicStatut::setStuff(StuffClient *co)
{
    controlCo = co;
    connect(controlCo, &StuffClient::connected, this, &SNESClassicStatut::onClientConnected);
    connect(controlCo, &StuffClient::disconnected, this, &SNESClassicStatut::onClientDisconnected);
    timer.start(2000);
    ui->infoLabel->setText(tr("Trying to connect to SNES Classic"));
}


QString SNESClassicStatut::readyString() const
{
    return tr("SNES classic ready for savestate");
}

QString SNESClassicStatut::unreadyString() const
{
    return tr("SNES classic not ready for savestate");
}

void SNESClassicStatut::stop()
{
    timer.stop();
}

void SNESClassicStatut::start()
{
    timer.start(2000);
}

#define CLOVERSAVESTATEPATH "/tmp/savestate2snes.svt"
#define CLOVERROLLBACKPATH "/tmp/rollback/"
#define SCREENSHOTPATH "/tmp/savestate2snes.png"

void SNESClassicStatut::onCanoeStarted()
{
    QByteArray ba = controlCo->waitForCommand("ps | grep canoe | grep -v grep");
    QString result = ba.trimmed();
    QString canoeStr = result.mid(result.indexOf("canoe"));
    QStringList canoeArgs = canoeStr.split(" ");
    if (canoeArgs.indexOf("-replay") != -1)
    {
        ui->romNameLabel->setText(tr("Canoe is on replay mode"));
        return ;
    }
    ui->infoLabel->setText(tr("Game started, waiting for init"));
    ui->romNameLabel->setText(canoeArgs.at(canoeArgs.indexOf("-rom") + 1));
    timer.stop();
    ui->iniButton->setEnabled(true);
    if (canoeArgs.indexOf(CLOVERSAVESTATEPATH) != -1)
    {
        emit readyForSaveState();
        ui->infoLabel->setText(tr("Ready for savestate"));
        ui->coStatusLabel->setPixmap(QPixmap(":/snesclassic status button green.png"));
    }
}

void SNESClassicStatut::onCanoeStopped()
{
    ui->romNameLabel->setText(tr("Canoe not running"));
    emit unReadyForSaveState();
}

void SNESClassicStatut::onClientConnected()
{
    sDebug() << "Control co connected";
    ui->coStatusLabel->setPixmap(QPixmap("://snesclassic status button orange.png"));
    ui->infoLabel->setText(tr("Waiting for a game to start"));
    checkForReady();
}

void SNESClassicStatut::onClientDisconnected()
{
    sDebug() << "Control co discconnected";
    ui->coStatusLabel->setPixmap(QPixmap(":/snesclassic status button red.png"));
    ui->infoLabel->setText(tr("Snes classic not running"));
    unReadyForSaveState();
}


void SNESClassicStatut::on_iniButton_clicked()
{
    if (!firstCanoeRun.isEmpty())
    {
        controlCo->waitForCommand("kill -9 `pidof canoe-shvc`");
        QThread::usleep(200);
        controlCo->detachedCommand(firstCanoeRun.toLatin1());
        return ;
    }

    QByteArray ba = controlCo->waitForCommand("ps | grep canoe | grep -v grep");
    QString result = ba.trimmed();
    QString canoeStr = result.mid(result.indexOf("canoe-shvc"));
    sDebug() << "Init pressed - Canoe Str : " << canoeStr;
    QStringList canoeRun = canoeStr.split(" ");
    if (canoeRun.indexOf("-rollback-output-dir") == -1)
    {
ready:
        firstCanoeRun = canoeRun.join(" ") + " 2>/dev/null";
        emit readyForSaveState();
        ui->infoLabel->setText(tr("Ready for savestate"));
        ui->iniButton->setText(tr("Reset", "Reset canoe run"));
        ui->coStatusLabel->setPixmap(QPixmap(":/snesclassic status button green.png"));
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
    QString canoePid = controlCo->waitForCommand("pidof canoe-shvc").trimmed();
    controlCo->waitForCommand(QString("kill -%1 `pidof canoe-shvc`").arg(2).toLatin1());
    controlCo->waitForCommand("test -e /tmp/plop || (sleep 1 && kill -2 `pidof ReedPlayer-Clover` && touch /tmp/plop)");
    controlCo->waitForCommand("wait " + canoePid.toLatin1());
    canoeRun.append("--save-on-quit");
    canoeRun.append(CLOVERSAVESTATEPATH);
    int sshot = canoeRun.indexOf("--save-screenshot-on-quit");
    canoeRun.removeAt(sshot);
    canoeRun.removeAt(sshot);
    canoeRun << "--save-screenshot-on-quit" << SCREENSHOTPATH;
    QThread::sleep(2);
    controlCo->detachedCommand(QString(canoeRun.join(" ") + " 2>/dev/null").toLatin1());
    goto ready;
}

void SNESClassicStatut::on_shortcutCheckBox_toggled(bool checked)
{
    emit shortcutsToggled(checked);
}
