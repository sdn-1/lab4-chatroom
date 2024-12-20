#include "chatclient.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

ChatClient::ChatClient(QObject *parent)
    : QObject{parent}
{
    m_clientSocket = new QTcpSocket(this);

    connect(m_clientSocket, &QTcpSocket::connected, this, &ChatClient::connected);
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &ChatClient::onReadyRead);
}

void ChatClient::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(m_clientSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);
    // start an infinite loop
    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {
            //emit messageReceived(QString::fromUtf8(jsonData));

            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject()) {
                    // emit logMessage(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact));
                    emit jsonReceived(jsonDoc.object());
                }
            }

        } else {
            break;
        }
    }
}

void ChatClient::sendMessage(const QString &text, const QString &type)
{
    if (m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if (!text.isEmpty()) {
        //create a QDataStream operating on the socket
        QDataStream serverSteam(m_clientSocket);
        serverSteam.setVersion(QDataStream::Qt_6_7);

        //create the JSON we want to send
        QJsonObject message;
        message["type"] = type;
        message["text"] = text;

        //send the JSON using QDataStream
        serverSteam << QJsonDocument(message).toJson();
    }
}

void ChatClient::connectToServer(const QHostAddress &address, quint16 port)
{
    m_clientSocket->connectToHost(address, port);
}

void ChatClient::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}
