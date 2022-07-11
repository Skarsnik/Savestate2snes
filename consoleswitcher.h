#ifndef CONSOLESWITCHER_H
#define CONSOLESWITCHER_H

#include "usb2snes.h"
#include "handlestuff.h"
#include "handlestuffsnesclassic.h"
#include "handlestuffusb2snes.h"
#include "handlestuffnwaccess.h"
#include "snesclassicstuff/stuffclient/stuffclient.h"
#include "emunwaccessclient.h"


#include <QWidget>

#include <snesclassicstuff/desktopclient/inputdecoder.h>

namespace Ui {
class ConsoleSwitcher;
}

class ConsoleSwitcher : public QWidget
{
    Q_OBJECT

public:
    enum Mode {
        USB2Snes,
        SNESClassic,
        NWAccess
    };
    Q_ENUMS(Mode)
    explicit ConsoleSwitcher(QWidget *parent = nullptr);
    HandleStuff*    getHandle();
    void            start();
    Mode            mode();
    QString         readyString() const;
    QString         unreadyString() const;
    ~ConsoleSwitcher();

public slots:
    void    refreshShortcuts();

signals:
    void    modeChanged(ConsoleSwitcher::Mode mode);
    void    readyForSaveState();
    void    unReadyForSaveState();

private slots:
    void    on_snesClassicInputDecoderButtonPressed(InputDecoder::SNESButton);
    void    on_snesClassicInputDecoderButtonReleased(InputDecoder::SNESButton);
    void    on_snesClassicInputNewData(QByteArray data);
    void    on_snesClassicInputConnected();
    void    on_snesClassicShortcutsToggled(bool toggled);
    void    on_snesClassicReadyForSaveState();
    void    on_snesClassicUnReadyForSaveState();
    void    on_snesClassicButton_clicked();
    void    on_nwaReadyForSaveState();
    void    on_nwaUnReadyForSaveState();

    void on_usb2snesButton_clicked();

    void on_emunwaccessButton_clicked();

private:
    Ui::ConsoleSwitcher *ui;

    Mode                    m_mode;
    USB2snes*               usb2snes;
    EmuNWAccessClient*      nwaclient;
    StuffClient*            stuffControlCo;
    StuffClient*            stuffInput;
    InputDecoder*           snesClassicInputDecoder;
    HandleStuffSnesClassic* handleSNESClassic;
    HandleStuffUsb2snes*    handleUSB2Snes;
    HandleStuffNWAccess*    handleNWAccess;
    QSettings*              m_settings;
    QList<int>              snesClassicButtonPressed;
    QMap<int, int>          mapEnumToSNES;

    bool                    usb2snesInit;
    bool                    snesClassicInit;
    bool                    nwaccessInit;
    bool                    snesClassicShortcutActivated;
    bool                    snesClassicReady;

    void    initUsb2snes();
    void    initSnesClassic();
    void    initNWAccess();
    void    cleanUpUSB2Snes();
    void    cleanUpSNESClassic();
    void    cleanUpNWAccess();

};

#endif // CONSOLESWITCHER_H
