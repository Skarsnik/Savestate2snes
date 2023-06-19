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
    void    setSavedPreset(MemoryPreset preset);

signals:
    void    memoryPresetChanged(MemoryPreset preset);

public slots:
    void    onSavestateLoaded();
    void    onMemoryRequestDone(quint64);


private slots:
    void on_configPushButton_clicked();

    void on_memoryCheckcheckBox_stateChanged(int arg1);

private:
    Ui::TrainingTimer *ui;
    QTimer          timer;
    quint64         oldAddressValue;
    QDateTime       startedTime;
    HandleStuff*    handler;
    bool            readyToTime;
    bool            firstMemoryTick;
    bool            firstLoad;
    TrainingConfigDialog    configDialog;

    void    onTimerTick();
    void    setLabelTime(QLabel*    label);
};

#endif // TRAININGTIMER_H
