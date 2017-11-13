#include "usb2snes.h"
#include <QUrl>
//#include <QSerialPortInfo>

USB2snes::USB2snes() : QObject()
{
    m_state = None;
    portRequested = false;
    QObject::connect(&m_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onWebSocketTextReceived(QString)));
    QObject::connect(&m_webSocket, SIGNAL(connected()), this, SLOT(onWebSocketConnected()));
    QObject::connect(&m_webSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onWebSocketBinaryReceived(QByteArray)));
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
    return QString();
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

void USB2snes::onWebSocketTextReceived(QString message)
{
    qDebug() << "<<T" << message;
    if (portRequested)
    {
        QJsonDocument   jdoc = QJsonDocument::fromJson(message.toLatin1());
        if (!jdoc.object()["Results"].toArray().isEmpty())
        {
            m_port = jdoc.object()["Results"].toArray()[0].toString();
            portRequested = false;
            sendRequest("Attach", QStringList() << m_port);
            changeState(Ready);
        }
    }
}

void USB2snes::onWebSocketBinaryReceived(QByteArray message)
{
    qDebug() << "<<B" << message;
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

QByteArray USB2snes::getAddress(QString addr, unsigned int size)
{
    sendRequest("GetAddress", QStringList() << addr << QString::number(size, 16));
    return QByteArray();
}

USB2snes::State USB2snes::state()
{
    return m_state;
}
