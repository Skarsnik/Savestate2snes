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
    ui->coStatusLabel->setPixmap(QPixmap(":/snesclassic status button red.png"));
    ui->shortcutCheckBox->setChecked(m_settings->value("SNESClassicShortcuts").toBool());
    ui->iniButton->setEnabled(false);
}

void SNESClassicStatut::onTimerTick()
{
    sDebug() << "Timer tick" << m_state;
    if (m_state == CONNECTED)
    {
        changeState(CONNECTED);
    }
    if (m_state == NONE)
    {
        controlCo->connect();
    }
}

#define CLOVERSAVESTATEPATH "/tmp/savestate2snes.svt"
#define CLOVERROLLBACKPATH "/tmp/rollback/"
#define SCREENSHOTPATH "/tmp/savestate2snes.png"

void    SNESClassicStatut::changeState(MyState newState)
{
    sDebug() << "Changing state to " << newState;
    switch(newState)
    {
    case NONE:
    {
        sDebug() << "Control co discconnected";
        ui->coStatusLabel->setPixmap(QPixmap(":/snesclassic status button red.png"));
        ui->infoLabel->setText(tr("Snes classic not running"));
        unReadyForSaveState();
        break;
    }
    case CONNECTED:
    {
        controlCo->executeCommand("pidof canoe-shvc > /dev/null && echo 1 || echo 0");
        sDebug() << "Control co connected";
        ui->coStatusLabel->setPixmap(QPixmap("://snesclassic status button orange.png"));
        ui->infoLabel->setText(tr("Waiting for a game to start"));
        m_state = CHECKING_CANOE;
        break;
    }
    case CANOE_RUNNING:
    {
        controlCo->executeCommand("ps | grep canoe | grep -v grep");
        m_state = CHECKING_READY;
        break;
    }
    case READY:
    {
        m_state = READY;
        emit readyForSaveState();
        ui->infoLabel->setText(tr("Ready for savestate"));
        ui->iniButton->setText(tr("Reset", "Reset canoe run"));
        ui->coStatusLabel->setPixmap(QPixmap(":/snesclassic status button green.png"));
        break;
    }
    case DOING_INIT:
    {
        controlCo->executeCommand("ps | grep canoe | grep -v grep");
        m_state = DOING_INIT;
        break;
    }
    case DOING_RESET:
    {
        controlCo->executeCommand("kill -9 `pidof canoe-shvc`");
        QThread::usleep(200);
        m_state = FINISHING_INIT;
        break;
    }
    }
}

void    SNESClassicStatut::onCommandFinished(bool success)
{
    sDebug() << "Command finished" << success << m_state;
    switch(m_state)
    {
    case CHECKING_CANOE:
    {
        QByteArray  result = controlCo->commandDatas();
        bool oldcr = canoeRunning;
        sDebug() << "Is canoerunning?" << canoeRunning;
        //sDebug() << result << result.trimmed();
        canoeRunning = (result.trimmed() == "1");
        qDebug() << canoeRunning;
        if (oldcr == false && canoeRunning == true)
        {
            //emit canoeStarted();
            changeState(MyState::CANOE_RUNNING);
            break;
        }
        if (oldcr == true && canoeRunning == false)
        {
            ui->romNameLabel->setText(tr("Canoe not running"));
            emit unReadyForSaveState();
        }
        m_state = CONNECTED;
        break;
    }
    case CHECKING_READY:
    {
        QString result = controlCo->commandDatas().trimmed();
        QString canoeStr = result.mid(result.indexOf("canoe"));
        QStringList canoeArgs = canoeStr.split(" ");
        if (canoeArgs.indexOf("-replay") != -1)
        {
            ui->romNameLabel->setText(tr("Canoe is on replay mode"));
            m_state = CONNECTED;
            return ;
        }
        ui->infoLabel->setText(tr("Game started, waiting for init"));
        ui->romNameLabel->setText(canoeArgs.at(canoeArgs.indexOf("-rom") + 1));
        timer.stop();
        ui->iniButton->setEnabled(true);
        if (canoeArgs.indexOf(CLOVERSAVESTATEPATH) != -1)
        {
            changeState(MyState::READY);
        }
        break;
    }
    case DOING_INIT:
    {
        QString result = controlCo->commandDatas().trimmed();
        QString canoeStr = result.mid(result.indexOf("canoe-shvc"));
        sDebug() << "Init pressed - Canoe Str : " << canoeStr;
        QStringList canoeRun = canoeStr.split(" ");
        if (canoeRun.indexOf("-rollback-output-dir") == -1)
        {
            firstCanoeRun = canoeRun.join(" ") + " 2>/dev/null";
            changeState(READY);
            break;
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
        canoeRun.append("--save-on-quit");
        canoeRun.append(CLOVERSAVESTATEPATH);
        int sshot = canoeRun.indexOf("--save-screenshot-on-quit");
        canoeRun.removeAt(sshot);
        canoeRun.removeAt(sshot);
        canoeRun << "--save-screenshot-on-quit" << SCREENSHOTPATH;
        firstCanoeRun = canoeRun.join(" ") + " 2>/dev/null";
        controlCo->executeCommand("TMP_CANOE_PID=`pidof canoe-shvc`;"
                                  "kill -2 `pidof canoe-shvc`;"
                                  "test -e /tmp/plop || (sleep 1 && kill -2 `pidof ReedPlayer-Clover` && touch /tmp/plop);"
                                  "wait $TMP_CANOE_PID;sleep 2");
        m_state = FINISHING_INIT;
        break;
    }
    case DOING_RESET:
    {
        m_state = FINISHING_INIT;
        break;
    }
    case FINISHING_INIT:
    {
        controlCo->detachedCommand(QString(firstCanoeRun).toLatin1());
        changeState(READY);
        break;
    }
    }
}

SNESClassicStatut::~SNESClassicStatut()
{
    timer.stop();
    delete ui;
}

void SNESClassicStatut::setStuff(StuffClient *co)
{
    controlCo = co;
    connect(controlCo, &StuffClient::connected, this, [=]() {
        changeState(MyState::CONNECTED);
    });
    connect(controlCo, &StuffClient::disconnected, this, [=](){
        changeState(MyState::NONE);
    });
    connect(controlCo, &StuffClient::commandFinished, this, &SNESClassicStatut::onCommandFinished);
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



void SNESClassicStatut::on_iniButton_clicked()
{
    if (!firstCanoeRun.isEmpty())
    {
        changeState(DOING_RESET);
        return ;
    }
    changeState(DOING_INIT);
}

void SNESClassicStatut::on_shortcutCheckBox_toggled(bool checked)
{
    emit shortcutsToggled(checked);
}
