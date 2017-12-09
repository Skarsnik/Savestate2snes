#include "usb2snes.h"
#include <QUrl>
//#include <QSerialPortInfo>

USB2snes::USB2snes() : QObject()
{
    m_state = None;
    portRequested = false;
    versionRequested = false;
    QObject::connect(&m_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onWebSocketTextReceived(QString)));
    QObject::connect(&m_webSocket, SIGNAL(connected()), this, SLOT(onWebSocketConnected()));
    QObject::connect(&m_webSocket, SIGNAL(disconnected()), this, SLOT(onWebSocketDisconnected()));
    QObject::connect(&m_webSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onWebSocketBinaryReceived(QByteArray)));
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    m_clientVersion = "Nop";
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
    changeState(Connected);
    portRequested = true;
    sendRequest("DeviceList");

}

void USB2snes::onWebSocketDisconnected()
{
    changeState(None);
    emit disconnected();
}

void USB2snes::onWebSocketTextReceived(QString message)
{
    qDebug() << "<<T" << message;
    if (versionRequested)
    {
        versionRequested = false;
        QJsonDocument   jdoc = QJsonDocument::fromJson(message.toLatin1());
        QJsonArray jarray = jdoc.object()["Results"].toArray();
        if (!jarray.empty())
        {
            m_firmwareVersion = jarray[0].toString();
            qDebug() << m_firmwareVersion;
            //otherVersion = jarray[1].toString();
            changeState(Ready);
        }
        return;
    }
    if (portRequested)
    {
        QJsonDocument   jdoc = QJsonDocument::fromJson(message.toLatin1());
        if (!jdoc.object()["Results"].toArray().isEmpty())
        {
            timer.stop();
            m_port = jdoc.object()["Results"].toArray()[0].toString();
            portRequested = false;
            sendRequest("Attach", QStringList() << m_port);
            versionRequested = true;
            timer.start(500);
        } else {
            timer.start(1000);
        }
        return;
    }
}

void USB2snes::onWebSocketBinaryReceived(QByteArray message)
{
    qDebug() << "<<B" << message.toHex('-') << message;
    lastBinaryMessage = message;
    emit binaryMessageReceived();
}

void USB2snes::onTimerTick()
{
    if (portRequested)
    {
        sendRequest("DeviceList");
    }
    if (versionRequested)
    {
        sendRequest("Info");
        timer.stop();
    }
}


void USB2snes::sendRequest(QString opCode, QStringList operands, QStringList flags)
{
    QJsonArray      jOp;
    QJsonObject     jObj;

    jObj["Opcode"] = opCode;
    jObj["Space"] = "SNES";
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

QByteArray USB2snes::getAddress(unsigned int addr, unsigned int size)
{
    sendRequest("GetAddress", QStringList() << QString::number(addr, 16) << QString::number(size, 16));
    QEventLoop  loop;
    QObject::connect(this, SIGNAL(binaryMessageReceived()), &loop, SLOT(quit()));
    loop.exec();
    return lastBinaryMessage;
}

void USB2snes::setAddress(unsigned int addr, QByteArray data)
{
    sendRequest("PutAddress", QStringList() << QString::number(addr, 16) << QString::number(data.size(), 16));
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

QString USB2snes::clientVersion()
{
    return m_clientVersion;
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
