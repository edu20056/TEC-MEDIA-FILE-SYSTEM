#include "diskNode.hpp"

DiskNode::DiskNode(QObject *parent, const QString &host, quint16 port,
    const QString path, quint16 id) 
    : QObject(parent), socket(new QTcpSocket(this)), path(path), nodeID(id) {

    connect(socket, &QTcpSocket::connected, this, &DiskNode::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &DiskNode::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &DiskNode::onError);

    socket->connectToHost(host, port);
   if(!initPath()){ qWarning() << "!ERROR: Directory couldn't be created..."; }
}

// ======================== CONNECTION FUNCTIONS ============================================

void DiskNode::onConnected() {

    emit connectionStatusChanged(true);
    nodeInfo(); 
}

void DiskNode::onDisconnected() {

    emit connectionStatusChanged(false);
    nodeInfo(); 
}

void DiskNode::onError(QAbstractSocket::SocketError error) {

    Q_UNUSED(error);
    qDebug() << "Error de socket:" << socket->errorString();
}

bool DiskNode::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

// ============================ DEBUGGING  =================================

void DiskNode::nodeInfo() const {
    QString status = isConnected() ? "Connected" : "Disconnected";

    qDebug().noquote() << "\n================== DISK NODE INFO ==================";
    qDebug().noquote() << QString("Node ID       : %1").arg(nodeID);
    qDebug().noquote() << QString("Path          : %1").arg(path);
    qDebug().noquote() << QString("Status        : %1").arg(status);
    qDebug().noquote() << "===================================================\n";
}

// ============================ FILE MANAGEMENT =================================

bool DiskNode::initPath() {
    QDir baseDir(path);

    if (!baseDir.exists()) {
        if (!baseDir.mkpath(".")) return false;
    }

    QString folderName = QString("DiskNode_%1").arg(nodeID);
    QString fullPath = baseDir.filePath(folderName);

    QDir nodeDir(fullPath);

    if (!nodeDir.exists()) {
        if (baseDir.mkdir(folderName)) return true;
        else return false;
    } else return true;

    path = fullPath;
    return true;
}

// =========================== CONNECTION FUNCTIONS  ==============================

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