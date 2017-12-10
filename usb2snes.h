#ifndef USB2SNES_H
#define USB2SNES_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>


#define USB2SNESURL "ws://localhost:8080/"


class USB2snes : public QObject
{
    Q_OBJECT
public:
    enum Space
    {
        SNES,
        CMD
    };
    enum State {
        None,
        Connected,
        Ready
    };
    enum sd2snesState {
        sd2menu,
        RomRunning
    };
    Q_ENUM(State)
    Q_ENUM(sd2snesState)
    // Should be private, but allow for Qt to register the enum
    enum InternalState {
        INone,
        IConnected,
        DeviceListRequested,
        AttachSent,
        FirmwareVersionRequested,
        ServerVersionRequested,
        IReady
    };
    Q_ENUM(InternalState)

    USB2snes();
    QPair<QString, QString> autoFind();
    void                    usePort(QString port);
    QString                 getPort();
    QString                 getRomName();
    void                    connect();
    QByteArray              getAddress(unsigned int addr, unsigned int size, Space space = SNES);
    void                    setAddress(unsigned int addr, QByteArray data, Space space = SNES);
    State                   state();
    QString                 firmwareVersion();
    QString                 serverVersion();
    bool                    patchROM(QString patch);

signals:
    void    stateChanged();
    void    disconnected();
    void    binaryMessageReceived();
    void    romStarted();
    void    menuStarted();


private slots:
    void    onWebSocketConnected();
    void    onWebSocketDisconnected();
    void    onWebSocketTextReceived(QString message);
    void    onWebSocketBinaryReceived(QByteArray message);
    void    onTimerTick();


private:
    QWebSocket      m_webSocket;
    QString         m_port;
    State           m_state;
    sd2snesState    m_sd2snesState;
    QString         m_firmwareVersion;
    QString         m_serverVersion;
    InternalState   m_istate;
    QByteArray      lastBinaryMessage;
    unsigned int    requestedBinaryReadSize;

    QTimer          timer;

    void            sendRequest(QString opCode, QStringList operands = QStringList(), Space = SNES, QStringList flags = QStringList());
    void            changeState(State s);
    QStringList     getJsonResults(QString json);

};

#endif // USB2SNES_H
