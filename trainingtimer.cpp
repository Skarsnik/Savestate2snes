#include <QLoggingCategory>
#include <QMessageBox>

Q_LOGGING_CATEGORY(log_trainingTimer, "Training Timer")

#define sDebug() qCDebug(log_trainingTimer())

#include "trainingtimer.h"
#include "ui_trainingtimer.h"


TrainingTimer::TrainingTimer(QWidget *parent) : QWidget(parent), ui(new Ui::TrainingTimer)
{
    ui->setupUi(this);
    oldAddressValue = 0;
    timer.setInterval(20);
    connect(&timer, &QTimer::timeout, this, &TrainingTimer::onTimerTick);
    /*QTimer* piko = new QTimer(this);
    piko->setInterval(1000);
    connect(piko, &QTimer::timeout, this, [=]() {
        readyToTime = true;
        onSavestateLoaded();
    });
    piko->start();*/
    readyToTime = false;
    firstMemoryTick = true;
    firstLoad = true;
}

TrainingTimer::~TrainingTimer()
{

}

bool TrainingTimer::loadPreset(const QString &path)
{
    return configDialog.loadPresets(path);
}

void TrainingTimer::setHandler(HandleStuff *stuff)
{
    disconnect(this, &TrainingTimer::memoryPresetChanged, handler, &HandleStuff::setMemoryToWatch);
    handler = stuff;
    connect(this, &TrainingTimer::memoryPresetChanged, handler, &HandleStuff::setMemoryToWatch);
    connect(handler, &HandleStuff::loadStateFinished, this, &TrainingTimer::onSavestateLoaded);
    connect(handler, &HandleStuff::controllerLoadStateFinished, this, &TrainingTimer::onSavestateLoaded);
    connect(handler, &HandleStuff::gotMemoryValue, this, &TrainingTimer::onMemoryRequestDone);
    connect(handler, &HandleStuff::newGameLoaded, this, [=] {
        if (handler->hasMemoryWatch())
        {
            if (handler->gameInfos().memoryPreset.domain.isEmpty() == false)
            {
                configDialog.setPreset(handler->gameInfos().memoryPreset);
                ui->memoryCheckcheckBox->setEnabled(true);
            }
            ui->configPushButton->setEnabled(true);
        } else {
            ui->memoryCheckcheckBox->setEnabled(false);
            ui->configPushButton->setEnabled(false);
        }
    });

}


void TrainingTimer::onSavestateLoaded()
{
    setLabelTime(ui->lastLoadClock);
    readyToTime = true;
    timer.start();
    startedTime = QDateTime::currentDateTime();
    firstMemoryTick = true;
    if (firstLoad)
    {
        if (handler->hasMemoryWatch() && ui->memoryCheckcheckBox->isChecked())
        {
            sDebug() << "Starting memory watch";
            handler->startMemoryWatch();
        }
        firstLoad = false;
    }
    sDebug() << startedTime;
}

void TrainingTimer::onMemoryRequestDone(quint64 value)
{
    if (firstMemoryTick)
    {
        oldAddressValue = value;
        firstMemoryTick = false;
    }
    else {
        sDebug() << "Got Value : " << QString::number(value, 16) << " Old Value " << QString::number(oldAddressValue, 16);
        if (value != oldAddressValue)
        {
            setLabelTime(ui->memoryClock);
            oldAddressValue = value;
        }
    }
}

void TrainingTimer::onTimerTick()
{
    if (readyToTime)
        setLabelTime(ui->mainClock);
}

void    TrainingTimer::setLabelTime(QLabel* label)
{
    QTime time(0, 0 , 0);
    //sDebug() << "MS to" << startedTime.msecsTo(QDateTime::currentDateTime());
    time = time.addMSecs(startedTime.msecsTo(QDateTime::currentDateTime()));
    //sDebug() << time;
    label->setText(time.toString("mm:ss:zzz"));
}


void TrainingTimer::on_configPushButton_clicked()
{
    auto ok = configDialog.exec();
    if (ok == QDialog::Accepted)
    {
        emit memoryPresetChanged(configDialog.currentPreset());
    }
}


void TrainingTimer::on_memoryCheckcheckBox_stateChanged(int state)
{
    ui->memoryClock->setEnabled(state == Qt::Checked);
    handler->saveMemoryCheck(state == Qt::Checked);
    if (state != Qt::Checked)
    {
        ui->memoryClock->setText("-");
        if (handler->hasMemoryWatch())
            handler->stopMemoryWatch();
    } else {
        ui->memoryClock->setText("00:00:00");
        /*if (handler->hasMemoryWatch())
            handler->startMemoryWatch();*/
    }
}

