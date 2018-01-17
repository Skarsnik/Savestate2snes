#ifndef SNESCLASSICSTATUT_H
#define SNESCLASSICSTATUT_H

#include <QTimer>
#include <QWidget>
#include "snesclassicstuff/desktopclient/telnetconnection.h"
#include "snesclassicstuff/desktopclient/miniftp.h"

namespace Ui {
class SNESClassicStatut;
}

class SNESClassicStatut : public QWidget
{
    Q_OBJECT

public:
    explicit SNESClassicStatut(QWidget *parent = 0);
    ~SNESClassicStatut();
    setCommandCo(TelnetConnection* telco);

signals:
    void    readyForSaveState();
    void    unReadyForSaveState();
    void    canoeStarted();
    void    canoeStopped();

private slots:
    void    onCanoeStarted();
    void    onCanoeStopped();
    void    onCommandCoConnected();
    void    onCommandCoDisconnected();
    void    onCommandCoError(TelnetConnection::ConnectionError);
    void    onTimerTick();

private:
    Ui::SNESClassicStatut   *ui;
    TelnetConnection        *cmdCo;
    MiniFtp                 *miniFtp;
    QTimer                  timer;
    bool                    canoeRunning;
    bool                    ftpReady;
};

#endif // SNESCLASSICSTATUT_H
