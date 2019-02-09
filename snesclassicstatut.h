#ifndef SNESCLASSICSTATUT_H
#define SNESCLASSICSTATUT_H

#include <QSettings>
#include <QTcpSocket>
#include <QTimer>
#include <QWidget>

#include "stuffclient.h"

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

signals:
    void    readyForSaveState();
    void    unReadyForSaveState();
    void    canoeStarted();
    void    canoeStopped();
    void    shortcutsToggled(bool);

private slots:
    void    onCanoeStarted();
    void    onCanoeStopped();
    void    onClientConnected();
    void    onClientDisconnected();
    void    onTimerTick();
    void    on_iniButton_clicked();

    void    on_shortcutCheckBox_toggled(bool checked);

private:
    Ui::SNESClassicStatut   *ui;
    StuffClient*            controlCo;
    StuffClient*            inputCo;
    QString                 firstCanoeRun;
    QTimer                  timer;
    bool                    canoeRunning;
    QSettings*              m_settings;

    bool checkForReady();
};

#endif // SNESCLASSICSTATUT_H
