#ifndef USB2SNESSTATUT_H
#define USB2SNESSTATUT_H

#include "usb2snes.h"

#include <QWidget>

namespace Ui {
class USB2SnesStatut;
}

class USB2SnesStatut : public QWidget
{
    Q_OBJECT

public:
    explicit USB2SnesStatut(QWidget *parent = 0);
    ~USB2SnesStatut();
    void    setUsb2snes(USB2snes* usnes);

signals:
    void    readyForSaveState();
    void    unReadyForSaveState();

public slots:
    void on_patchROMpushButton_clicked();


private slots:
    void    onTimerTick();
    void    onUsb2snesStateChanged();
    void    onUsb2snesDisconnected();
    void    onRomStarted();

    void on_pushButton_clicked();

private:
    Ui::USB2SnesStatut *ui;
    USB2snes*           usb2snes;

    QTimer              timer;
    bool                isPatchedRom();
};

#endif // USB2SNESSTATUT_H
