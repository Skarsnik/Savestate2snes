#ifndef CONSOLESWITCHER_H
#define CONSOLESWITCHER_H

#include "usb2snes.h"
#include "handlestuff.h"
#include "handlestuffsnesclassic.h"
#include "handlestuffusb2snes.h"
#include "snesclassicstuff/stuffclient/stuffclient.h"


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
        SNESClassic
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
    void on_snesClassicButton_clicked();

    void on_usb2snesButton_clicked();

private:
    Ui::ConsoleSwitcher *ui;

    Mode                    m_mode;
    USB2snes*               usb2snes;
    StuffClient*            stuffControlCo;
    StuffClient*            stuffInput;
    InputDecoder*           snesClassicInputDecoder;
    HandleStuffSnesClassic* handleSNESClassic;
    HandleStuffUsb2snes*    handleUSB2Snes;
    QSettings*              m_settings;
    QList<int>              snesClassicButtonPressed;
    QMap<int, int>          mapEnumToSNES;

    bool                    usb2snesInit;
    bool                    snesClassicInit;
    bool                    snesClassicShortcutActivated;
    bool                    snesClassicReady;

    void    initUsb2snes();
    void    initSnesClassic();
    void    cleanUpUSB2Snes();
    void    cleanUpSNESClassic();

};

#endif // CONSOLESWITCHER_H
