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
    setCommandCo(TelnetConnection* telco, TelnetConnection *canoe);
    setFtp(MiniFtp* ftp);
    QString readString() const;
    QString unreadyString() const;

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
    void    onMiniFTPConnected();

    void on_iniButton_clicked();

private:
    Ui::SNESClassicStatut   *ui;
    TelnetConnection        *cmdCo;
    TelnetConnection        *canoeCo;
    MiniFtp                 *miniFtp;
    QTimer                  timer;
    bool                    canoeRunning;
    bool                    ftpReady;

    bool checkForReady();
};

#endif // SNESCLASSICSTATUT_H
