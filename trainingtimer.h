#ifndef TRAININGTIMER_H
#define TRAININGTIMER_H

#include "handlestuff.h"

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
    void    setHandler(HandleStuff* stuff);
    void    setMemoryInfo(quint64 address, quint8 size);

signals:

public slots:
    void    onSavestateLoaded();
    void    onMemoryRequestDone(quint64);

private slots:
    void on_addressLineEdit_editingFinished();

private:
    Ui::TrainingTimer *ui;
    QTimer          timer;
    quint64         oldAddressValue;
    QDateTime       startedTime;
    HandleStuff*    handler;
    bool            readyToTime;
    bool            firstMemoryTick;
    bool            firstLoad;

    void    onTimerTick();
    void    setLabelTime(QLabel*    label);
};

#endif // TRAININGTIMER_H
