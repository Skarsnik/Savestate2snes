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
    memoryWatchActive = false;
    saveStateStartedTime = QDateTime::currentDateTime();
    memoryStartedTime = QDateTime::currentDateTime();
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
                ui->memoryCheckcheckBox->setChecked(true);
                ui->memoryCheckcheckBox->setText(tr("Check a change of memory value"));
            } else {
                ui->memoryCheckcheckBox->setText(tr("Open the config dialog to set preset"));
            }
            ui->configPushButton->setEnabled(true);
        } else {
            ui->memoryCheckcheckBox->setText(tr("No support for memory read"));
            ui->memoryCheckcheckBox->setEnabled(false);
            ui->configPushButton->setEnabled(false);
        }
    });

}


void TrainingTimer::onSavestateLoaded()
{
    setLabelTime(ui->lastLoadClock, saveStateStartedTime);
    readyToTime = true;
    timer.start();
    saveStateStartedTime = QDateTime::currentDateTime();
    firstMemoryTick = true;
    sDebug() << saveStateStartedTime;
}

void TrainingTimer::onMemoryRequestDone(quint64 value)
{
    if (firstMemoryTick)
    {
        oldAddressValue = value;
        firstMemoryTick = false;
    }
    else {
        //sDebug() << "Got Value : " << QString::number(value, 16) << " Old Value " << QString::number(oldAddressValue, 16);
        if (value != oldAddressValue)
        {
            sDebug() << "Value changed";
            setLabelTime(ui->memoryClock, memoryStartedTime);
            memoryStartedTime = QDateTime::currentDateTime();
            oldAddressValue = value;
        }
    }
}

void TrainingTimer::saveStateReady()
{
    sDebug() << "Savestate ready";
    ui->lastLoadClock->setEnabled(true);
    ui->memoryClock->setEnabled(true);
    ui->memoryCheckcheckBox->setEnabled(true);
    if (handler->hasMemoryWatch() && ui->memoryCheckcheckBox->isChecked())
    {
        startMemoryWatch();
    }
}

void TrainingTimer::saveStateUnready()
{
    ui->lastLoadClock->setEnabled(false);
    ui->memoryClock->setEnabled(false);
}

void    TrainingTimer::startMemoryWatch()
{
    firstMemoryTick = true;
    sDebug() << "Starting memory watch";
    memoryWatchActive = true;
    readyToTime = true;
    timer.start();
    handler->startMemoryWatch();
    memoryStartedTime = QDateTime::currentDateTime();
}

void    TrainingTimer::stopMemoryWatch()
{
    sDebug() << "Stoping memory watch";
    memoryWatchActive = false;
    handler->stopMemoryWatch();
}

void TrainingTimer::onTimerTick()
{
    if (readyToTime)
    {
        if (memoryWatchActive)
            setLabelTime(ui->mainClock, memoryStartedTime);
        else
            setLabelTime(ui->mainClock, saveStateStartedTime);
    }
}

void    TrainingTimer::setLabelTime(QLabel* label, QDateTime& startedTime)
{
    QTime time(0, 0 , 0);
    //sDebug() << "MS to" << saveStateStartedTime.msecsTo(QDateTime::currentDateTime());
    time = time.addMSecs(startedTime.msecsTo(QDateTime::currentDateTime()));
    //sDebug() << time;
    QString timeText = QString(time.toString("mm:ss:zzz"));
    timeText.truncate(8);
    label->setText(timeText);
}


void TrainingTimer::on_configPushButton_clicked()
{
    auto ok = configDialog.exec();
    if (ok == QDialog::Accepted)
    {
        sDebug() << "Selected preset " << configDialog.currentPreset().address;
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
        if (handler->hasMemoryWatch() && ui->lastLoadClock->isEnabled())
            stopMemoryWatch();
    } else {
        ui->memoryClock->setText("00:00:00");
        if (handler->hasMemoryWatch() && ui->lastLoadClock->isEnabled())
            startMemoryWatch();
    }
}

