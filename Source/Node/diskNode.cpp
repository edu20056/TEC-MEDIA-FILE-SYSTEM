#include "diskNode.hpp"
#include <QDataStream>

DiskNode::DiskNode(QObject *parent, const QString &host, quint16 port,
    const QString path, quint16 id) 
    : QObject(parent), socket(new QTcpSocket(this)), path(path), nodeID(id), messageFormat(3) {

    connect(socket, &QTcpSocket::readyRead, this, &DiskNode::onReadyRead);
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

    if (!baseDir.exists() && !baseDir.mkpath(".")) {
        return false;
    }

    QString folderName = QString("DiskNode_%1").arg(nodeID);
    QString fullPath = baseDir.filePath(folderName);
    QDir nodeDir(fullPath);

    if (!nodeDir.exists() && !baseDir.mkdir(folderName)) {
        return false;
    }

    path = fullPath;
    qDebug().noquote() << QString("Path          : %1").arg(path);

    return true;
}

// =========================== CONNECTION FUNCTIONS  ==============================

// En la clase DiskNode (declaración como miembro privado)
QHash<QTcpSocket*, QByteArray> buffers;  // Buffer por cliente

void DiskNode::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    // Acumulate data on buffer.
    buffers[socket] += socket->readAll();


    while (buffers[socket].size() >= static_cast<qsizetype>(sizeof(quint32))) {
        QDataStream stream(buffers[socket]);
        quint32 messageLength;
        stream >> messageLength; 

        if (buffers[socket].size() - sizeof(quint32) >= messageLength) {
            // Extraer el mensaje completo (sin el prefijo)
            QByteArray completeMessage = buffers[socket].mid(sizeof(quint32), messageLength);
            buffers[socket].remove(0, sizeof(quint32) + messageLength);

            messageFormat.readMessage(completeMessage);
            qDebug() << "[DiskNode] Mensaje recibido - Indicador:" << messageFormat.getIndicator();
            qDebug() << "Nombre del archivo:" << messageFormat.getFileName();
            qDebug() << "Tamaño del contenido:" << messageFormat.getContent().size();

            // Opcional: Imprimir primeros bytes del contenido (hex)
            if (!messageFormat.getContent().isEmpty()) {
                qDebug() << "Primeros 50 bytes (hex):" << messageFormat.getContent().left(50).toHex(' ');
            }
        } else {
            break;  
        }
    }
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