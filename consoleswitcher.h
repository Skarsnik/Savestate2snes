#ifndef CONSOLESWITCHER_H
#define CONSOLESWITCHER_H

#include "usb2snes.h"
#include "snesclassicstuff/desktopclient/telnetconnection.h"
#include "snesclassicstuff/desktopclient/miniftp.h"
#include "handlestuff.h"
#include "handlestuffsnesclassic.h"
#include "handlestuffusb2snes.h"


#include <QWidget>

namespace Ui {
class ConsoleSwitcher;
}

class ConsoleSwitcher : public QWidget
{
    Q_OBJECT

    enum Mode {
        USB2Snes,
        SNESClassic
    };

public:
    explicit ConsoleSwitcher(QWidget *parent = 0);
    HandleStuff*    getHandle();
    void            start();
    Mode            mode();
    ~ConsoleSwitcher();

public slots:
    void    refreshShortcuts();

signals:
    void    modeChanged(Mode mode);
    void    readyForSaveState();
    void    unReadyForSaveState();

private:
    Ui::ConsoleSwitcher *ui;

    Mode                    m_mode;
    USB2snes*               usb2snes;
    TelnetConnection*       telnetCommandCo;
    TelnetConnection*       telnetCanoeCo;
    TelnetConnection*       telnetInputCo;
    HandleStuffSnesClassic* handleSNESClassic;
    HandleStuffUsb2snes*    handleUSB2Snes;
    MiniFtp*                miniFTP;
    QSettings*              m_settings;

    bool                    usb2snesInit;
    bool                    snesClassicInit;

    void    initUsb2snes();
    void    initSnesClassic();

};

#endif // CONSOLESWITCHER_H
