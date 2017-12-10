#include "usb2snes.h"
#include <QUrl>
//#include <QSerialPortInfo>

USB2snes::USB2snes() : QObject()
{
    m_state = None;
    m_istate = INone;

    QObject::connect(&m_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onWebSocketTextReceived(QString)));
    QObject::connect(&m_webSocket, SIGNAL(connected()), this, SLOT(onWebSocketConnected()));
    QObject::connect(&m_webSocket, SIGNAL(disconnected()), this, SLOT(onWebSocketDisconnected()));
    QObject::connect(&m_webSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onWebSocketBinaryReceived(QByteArray)));
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    m_serverVersion = "Nop";
    requestedBinaryReadSize = 0;
}

QPair<QString, QString> USB2snes::autoFind()
{
    QPair<QString, QString> toret;

    return toret;
}

void    USB2snes::usePort(QString port)
{
    m_port = port;
}

QString USB2snes::getPort()
{
    return m_port;
}

QString USB2snes::getRomName()
{
    //Need to gather data then check if this is consistnt for lorom, otherwise it's hirom
    QByteArray title = getAddress(0x7FC0, 21);
    QByteArray romMakeUp = getAddress(0x7FD5, 1);
    //QByteArray tmp = getAddress(0xC0FFD5, 2);

    if (romMakeUp[0] & 0x1)
    {
        title = getAddress(0xFFC0, 21);
    }
    return QString(title);
}

void USB2snes::connect()
{
    if (m_state == None)
        m_webSocket.open(QUrl(USB2SNESURL));
}

void USB2snes::onWebSocketConnected()
{
    qDebug() << "Websocket connected";
    changeState(Connected);
    m_istate = IConnected;
    m_istate = DeviceListRequested;
    sendRequest("DeviceList");
}

void USB2snes::onWebSocketDisconnected()
{
    qDebug() << "Websocket disconnected";
    changeState(None);
    m_istate = INone;
    emit disconnected();
}

QStringList USB2snes::getJsonResults(QString json)
{
    QStringList toret;
    QJsonDocument   jdoc = QJsonDocument::fromJson(json.toLatin1());
    if (!jdoc.object()["Results"].toArray().isEmpty())
    {
        QJsonArray jarray = jdoc.object()["Results"].toArray();
        foreach(QVariant entry, jarray.toVariantList())
        {
            toret << entry.toString();
        }
    }
    return toret;
}

void USB2snes::onWebSocketTextReceived(QString message)
{
    qDebug() << "<<T" << message;
    switch (m_istate)
    {
    case DeviceListRequested:
    {
        QStringList results = getJsonResults(message);
        if (!results.isEmpty())
        {
            timer.stop();
            m_port = results.at(0);
            sendRequest("Attach", QStringList() << m_port);
            m_istate = AttachSent;
            timer.start(200);
        } else {
            timer.start(1000);
        }
        break;
    }
    case FirmwareVersionRequested:
    {
        QStringList results = getJsonResults(message);
        if (!results.isEmpty())
        {
            m_firmwareVersion = results.at(0);
            m_istate = ServerVersionRequested;
            sendRequest("AppVersion");
        }
        break;
    }
    case ServerVersionRequested:
    {
        QStringList results = getJsonResults(message);
        if (!results.isEmpty())
        {
            m_serverVersion = results.at(0);
            m_istate = IReady;
            changeState(Ready);
        }
        break;
    }
    }
}

void USB2snes::onWebSocketBinaryReceived(QByteArray message)
{
    static QByteArray buffer;
    if (message.size() < 100)
      qDebug() << "<<B" << message.toHex('-') << message;
    else
      qDebug() << "<<B" << "Received " << message.size() << " byte of data";
    buffer.append(message);
    if (buffer.size() == requestedBinaryReadSize)
    {
        lastBinaryMessage = buffer;
        emit binaryMessageReceived();
        buffer.clear();
    }
}

void USB2snes::onTimerTick()
{
    if (m_istate == AttachSent)
    {
        sendRequest("Info");
        m_istate = FirmwareVersionRequested;
        timer.stop();
    }
    if (m_istate == DeviceListRequested)
    {
        sendRequest("DeviceList");
    }
}


void USB2snes::sendRequest(QString opCode, QStringList operands, Space space, QStringList flags)
{
    QJsonArray      jOp;
    QJsonObject     jObj;

    jObj["Opcode"] = opCode;
    if (space == SNES)
        jObj["Space"] = "SNES";
    if (space == CMD)
        jObj["Space"] = "CMD";
    foreach(QString sops, operands)
        jOp.append(QJsonValue(sops));
    if (!operands.isEmpty())
        jObj["Operands"] = jOp;
    qDebug() << ">>" << QJsonDocument(jObj).toJson();
    m_webSocket.sendTextMessage(QJsonDocument(jObj).toJson());
}

void USB2snes::changeState(USB2snes::State s)
{
    m_state = s;
    emit stateChanged();
}

QByteArray USB2snes::getAddress(unsigned int addr, unsigned int size, Space space)
{
    sendRequest("GetAddress", QStringList() << QString::number(addr, 16) << QString::number(size, 16), space);
    requestedBinaryReadSize = size;
    QEventLoop  loop;
    QObject::connect(this, SIGNAL(binaryMessageReceived()), &loop, SLOT(quit()));
    loop.exec();
    requestedBinaryReadSize = 0;
    return lastBinaryMessage;
}

void USB2snes::setAddress(unsigned int addr, QByteArray data, Space space)
{
    sendRequest("PutAddress", QStringList() << QString::number(addr, 16) << QString::number(data.size(), 16), space);
    m_webSocket.sendBinaryMessage(data);
}

USB2snes::State USB2snes::state()
{
    return m_state;
}

QString USB2snes::firmwareVersion()
{
    return m_firmwareVersion;
}

QString USB2snes::serverVersion()
{
    return m_serverVersion;
}

bool USB2snes::patchROM(QString patch)
{
    QFile fPatch(patch);
    if (fPatch.open(QIODevice::ReadOnly))
    {
        unsigned int size = fPatch.size();
        sendRequest("PutIPS", QStringList() << "hook" << QString::number(size, 16));
        QByteArray data = fPatch.readAll();
        m_webSocket.sendBinaryMessage(data);
        return true;
    }
    return false;
}
