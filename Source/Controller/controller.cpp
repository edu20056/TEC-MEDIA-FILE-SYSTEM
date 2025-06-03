#include "controller.hpp"
#include <QDebug>

NodeController::NodeController(QObject *parent, quint16 port) : QTcpServer(parent){

    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Server could not start:" << errorString();
    } else {
        qInfo() << "Server listening on port" << port;
    }
}

void NodeController::incomingConnection(qintptr socketDescriptor){

    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);
    
    connect(client, &QTcpSocket::readyRead, this, &NodeController::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &NodeController::onDisconnected);
    connect(client, &QTcpSocket::disconnected, client, &QTcpSocket::deleteLater);

    clients.append(client);
    qInfo() << "New client connected:" << client->peerAddress().toString();
}

void NodeController::onReadyRead(){

    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    QByteArray data = client->readAll();
    emit dataReceived(client, data);
    qDebug() << "Received from client:" << data;
}

void NodeController::onDisconnected(){

    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    clients.removeAll(client);
    qInfo() << "Client disconnected:" << client->peerAddress().toString();
}

void NodeController::sendData(QTcpSocket *client, const QByteArray &data){

    if (client && client->state() == QAbstractSocket::ConnectedState) {
        client->write(data);
        client->waitForBytesWritten(1000);
    }
}