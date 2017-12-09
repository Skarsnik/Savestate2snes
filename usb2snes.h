#ifndef USB2SNES_H
#define USB2SNES_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>


#define USB2SNESURL "ws://localhost:8080/"


class USB2snes : public QObject
{
    Q_OBJECT
public:
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

    USB2snes();
    QPair<QString, QString> autoFind();
    void                    usePort(QString port);
    QString                 getPort();
    QString                 getRomName();
    void                    connect();
    QByteArray              getAddress(unsigned int addr, unsigned int size);
    void                    setAddress(unsigned int addr, QByteArray data);
    State                   state();
    QString                 firmwareVersion();
    QString                 clientVersion();
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
    QString         m_clientVersion;
    QByteArray      lastBinaryMessage;

    bool            portRequested;
    bool            versionRequested;
    QTimer          timer;

    void    sendRequest(QString opCode, QStringList operands = QStringList(), QStringList flags = QStringList());
    void    changeState(State s);

};

#endif // USB2SNES_H
