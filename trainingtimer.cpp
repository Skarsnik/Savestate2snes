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

void TrainingTimer::setHandler(HandleStuff *stuff)
{
    handler = stuff;
    //ui->memoryClock->setEnabled(handler->hasMemoryWatch());
    connect(handler, &HandleStuff::loadStateFinished, this, &TrainingTimer::onSavestateLoaded);
    connect(handler, &HandleStuff::gotMemoryValue, this, &TrainingTimer::onMemoryRequestDone);
}

void TrainingTimer::setMemoryInfo(quint64 address, quint8 size)
{
    ui->addressLineEdit->setText(QString("%1:%2").arg(size).arg(address, 0, 16));
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
        sDebug() << "Starting memory watch";
        if (handler->hasMemoryWatch())
            handler->startMemoryWatch();
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
    label->setText(time.toString("mm::ss::zzz"));
}

void TrainingTimer::on_addressLineEdit_editingFinished()
{
    sDebug() << "Address changed" << ui->addressLineEdit->text();
    auto info = ui->addressLineEdit->text().split(":");
    if (info.size() != 2)
        QMessageBox::warning(this, tr("Invalid format"), tr("Invalid format for the address information"));
    else {
        bool ok;
        quint64 add = info.at(1).toLong(&ok, 16);
        quint8 size = info.at(0).toInt(&ok);
        sDebug() << "Address : " << add << " size : " << size;
        if (!ok)
            QMessageBox::warning(this, tr("Invalid format"), tr("Invalid format for the address information"));
        else {
            handler->setMemoryToWatch(add, size);
            handler->saveMemoryInfos(add, size);
        }
    }
}
