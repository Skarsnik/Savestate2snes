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
    void    setCommandCo(TelnetConnection* telco, TelnetConnection *canoe);
    void    setFtp(MiniFtp* ftp);
    QString readyString() const;
    QString unreadyString() const;

signals:
    void    readyForSaveState();
    void    unReadyForSaveState();
    void    canoeStarted();
    void    canoeStopped();
    void    shortcutsToggled(bool);

private slots:
    void    onCanoeStarted();
    void    onCanoeStopped();
    void    onCommandCoConnected();
    void    onCommandCoDisconnected();
    void    onCommandCoError(TelnetConnection::ConnectionError);
    void    onTimerTick();
    void    onMiniFTPConnected();

    void    on_iniButton_clicked();

    void    on_shortcutCheckBox_toggled(bool checked);

private:
    Ui::SNESClassicStatut   *ui;
    TelnetConnection        *cmdCo;
    TelnetConnection        *canoeCo;
    TelnetConnection        *inputCo;
    MiniFtp                 *miniFtp;
    QString                 firstCanoeRun;
    QTimer                  timer;
    bool                    canoeRunning;
    bool                    ftpReady;

    bool checkForReady();
};

#endif // SNESCLASSICSTATUT_H
