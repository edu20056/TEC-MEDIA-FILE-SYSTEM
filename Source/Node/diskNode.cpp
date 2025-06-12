#include "diskNode.hpp"

DiskNode::DiskNode(QObject *parent, const QString &host, quint16 port) : QObject(parent), socket(new QTcpSocket(this)), messageFormat(3) {

    connect(socket, &QTcpSocket::readyRead, this, &DiskNode::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &DiskNode::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &DiskNode::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &DiskNode::onError);

    qDebug() << "Connecting to server at" << host << ":" << port;
    socket->connectToHost(host, port);
}

// ======================== CONNECTION FUNCTIONS ============================================

void DiskNode::onConnected() {

    qDebug() << "Conectado al servidor";
    emit connectionStatusChanged(true);
}

void DiskNode::onDisconnected() {

    qDebug() << "Desconectado del servidor";
    emit connectionStatusChanged(false);
}

void DiskNode::onError(QAbstractSocket::SocketError error) {

    Q_UNUSED(error);
    qDebug() << "Error de socket:" << socket->errorString();
}

bool DiskNode::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}


void DiskNode::onReadyRead(){

    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    qDebug() << "MENSAJE PRUEBA (esto es en diskNode onReadyRead)";
    QByteArray data = socket->readAll();
    messageFormat.readMessage(data);
    qDebug() << "Received from client, Indicator message:" << messageFormat.getIndicator();
}

// =========================== SEND INFORMATION ==============================================

void DiskNode::sendData(const QByteArray &data) {

    if (!isConnected()) {
        qDebug() << "No conectado, no se puede enviar datos";
        return;
    }
    
    qint64 bytesWritten = socket->write(data);
    if (bytesWritten == -1) {
        qDebug() << "Error al enviar datos:" << socket->errorString();
        return;
    }
    
    if (!socket->waitForBytesWritten(1000)) {
        qDebug() << "Timeout al enviar datos";
    }
}