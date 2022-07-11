#ifndef SNESCLASSICSTATUT_H
#define SNESCLASSICSTATUT_H

#include <QSettings>
#include <QTcpSocket>
#include <QTimer>
#include <QWidget>

#include "snesclassicstuff/stuffclient/stuffclient.h"

namespace Ui {
class SNESClassicStatut;
}

class SNESClassicStatut : public QWidget
{
    Q_OBJECT

public:
    explicit SNESClassicStatut(QWidget *parent = nullptr);
    ~SNESClassicStatut();
    void    setStuff(StuffClient* co);
    QString readyString() const;
    QString unreadyString() const;
    void    stop();
    void    start();

    enum MyState {
        NONE,
        CONNECTED, // Connected
        CHECKING_CANOE, // checking if canoe is running
        CANOE_RUNNING, // Canoe is running
        CHECKING_READY, // Checking if canoe is ready
        DOING_INIT, // Initialize canoe in the savestate mode for the first time
        FINISHING_INIT, // When it's done
        DOING_RESET, // When reseting an already ready canoe run
        READY
    };
    Q_ENUM(MyState)

signals:
    void    readyForSaveState();
    void    unReadyForSaveState();
    void    shortcutsToggled(bool);

private slots:
    void    onTimerTick();
    void    on_iniButton_clicked();

    void    on_shortcutCheckBox_toggled(bool checked);

    void    onCommandFinished(bool success);

private:
    Ui::SNESClassicStatut   *ui;
    StuffClient*            controlCo;
    StuffClient*            inputCo;
    QString                 firstCanoeRun;
    QTimer                  timer;
    bool                    canoeRunning;
    QSettings*              m_settings;
    MyState                 m_state;

    bool                    checkForReady();
    void                    changeState(MyState state);
};

#endif // SNESCLASSICSTATUT_H
