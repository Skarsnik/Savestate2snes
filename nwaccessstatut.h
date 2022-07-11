#ifndef NWACCESSSTATUT_H
#define NWACCESSSTATUT_H

#include <QTimer>
#include <QWidget>
#include <emunwaccessclient.h>

namespace Ui {
class NWAccessStatut;
}

class NWAccessStatut : public QWidget
{
    Q_OBJECT

public:
    explicit NWAccessStatut(QWidget *parent = nullptr);
    ~NWAccessStatut();
    void    start();
    void    stop();
    QString readyString();
    QString unreadyString();


signals:
    void    readyForSaveState();
    void    unReadyForSaveState();

private:
    Ui::NWAccessStatut *ui;
    EmuNWAccessClient*  client;
    bool                checkingInfo;
    bool                checkingStatus;
    QTimer              checkStatusTimer;
    void                onTimerTimeout();
};

#endif // NWACCESSSTATUT_H
