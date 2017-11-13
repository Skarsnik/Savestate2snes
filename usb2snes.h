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


    USB2snes();
    QPair<QString, QString> autoFind();
    void                    usePort(QString port);
    QString                 getPort();
    QString                 getRomName();
    void                    connect();
    QByteArray              getAddress(QString addr, unsigned int size);
    State                   state();

signals:
    void    stateChanged();


private slots:
    void    onWebSocketConnected();
    void    onWebSocketTextReceived(QString message);
    void    onWebSocketBinaryReceived(QByteArray message);


private:
    QWebSocket      m_webSocket;
    QString         m_port;
    State           m_state;
    sd2snesState    m_sd2snesState;

    bool            portRequested;

    void    sendRequest(QString opCode, QStringList operands = QStringList(), QStringList flags = QStringList());
    void    changeState(State s);

};

#endif // USB2SNES_H
