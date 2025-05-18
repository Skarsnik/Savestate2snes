#ifndef TRAININGTIMER_H
#define TRAININGTIMER_H

#include "handlestuff.h"
#include "trainingconfigdialog.h"

#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QLabel>

namespace Ui {
class TrainingTimer;
}

class TrainingTimer : public QWidget
{
    Q_OBJECT

public:
    explicit TrainingTimer(QWidget *parent = nullptr);
    ~TrainingTimer();
    bool    loadPreset(const QString& path);
    void    setHandler(HandleStuff* stuff);

signals:
    void    memoryPresetChanged(MemoryPreset preset);

public slots:
    void    onSavestateLoaded();
    void    onMemoryRequestDone(quint64);
    void    saveStateReady();
    void    saveStateUnready();


private slots:
    void on_configPushButton_clicked();

    void on_memoryCheckcheckBox_stateChanged(int arg1);

private:
    Ui::TrainingTimer *ui;
    QTimer          timer;
    quint64         oldAddressValue;
    QDateTime       saveStateStartedTime;
    QDateTime       memoryStartedTime;
    HandleStuff*    handler;
    bool            readyToTime;
    bool            firstMemoryTick;
    bool            firstLoad;
    bool            memoryWatchActive;
    TrainingConfigDialog    configDialog;

    void    onTimerTick();
    void    setLabelTime(QLabel*    label, QDateTime &startedTime);
    void    startMemoryWatch();
    void    stopMemoryWatch();
};

#endif // TRAININGTIMER_H
