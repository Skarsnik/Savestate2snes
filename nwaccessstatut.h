#ifndef NWACCESSSTATUT_H
#define NWACCESSSTATUT_H

#include <QSettings>
#include <QTimer>
#include <QWidget>
#include <emunwaccessclient.h>
#include <Button-Mash/localcontroller.h>

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
    void    setSettings(QSettings* set);
    void    setToShow();
    LocalController* localController;
    LocalControllerMapping  mapping;


signals:
    void    readyForSaveState();
    void    unReadyForSaveState();
    void    localControllerChanged();

private slots:
    void on_refreshPushButton_clicked();

    void on_mappingButton_clicked();

private:
    Ui::NWAccessStatut *ui;
    EmuNWAccessClient*  client;
    bool                checkingInfo;
    bool                checkingStatus;
    QSettings*          settings;
    QTimer              checkStatusTimer;
    void                onTimerTimeout();
    void                listController();
    QList<LocalControllerInfos> controllersInfos;
};

#endif // NWACCESSSTATUT_H
