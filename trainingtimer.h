#ifndef TRAININGTIMER_H
#define TRAININGTIMER_H

#include <QTimer>
#include <QWidget>

namespace Ui {
class TrainingTimer;
}

class TrainingTimer : public QWidget
{
    Q_OBJECT
public:
    explicit TrainingTimer(QWidget *parent = nullptr);

signals:

public slots:

private:
    Ui::TrainingTimer *ui;
    QTimer  timer;
    quint32 oldAddressValue;
};

#endif // TRAININGTIMER_H
