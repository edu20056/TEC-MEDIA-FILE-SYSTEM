#include "diskNode.hpp"
#include <QDataStream>

DiskNode::DiskNode(QObject *parent, const QString &host, quint16 port,
    const QString path, quint16 id) 
    : QObject(parent), socket(new QTcpSocket(this)), path(path), nodeID(id) {

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

void DiskNode::inputNotification(httpFormat const &messageData, QString fileName) const {
    QString msgStatus = messageData.getContent().isEmpty() ? "Empty File" : "Not Empty File";

    qDebug().noquote() << "\n=================== CTRL INPUT ===================";
    qDebug().noquote() << QString("File Name      : %1").arg(fileName);
    qDebug().noquote() << QString("File Stats     : %1").arg(msgStatus);
    qDebug().noquote() << QString("Size           : %1").arg(messageData.getContent().size());
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
    return true;
}

bool DiskNode::storeFile(const QByteArray& data, QString fileName) {

    QDir dir(path);
    if (!dir.exists()) return false;

    QString filePath = dir.absoluteFilePath(fileName);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    qint64 bytesWritten = file.write(data);
    if (bytesWritten == -1) return false;
    file.close();

    return reconstructPdf(data, fileName); 
}

bool DiskNode::reconstructPdf(const QByteArray& pdfData, const QString& fileName) {
    QDir dir(path);
    if (!dir.exists()) return false;

    QString safeFileName = fileName;
    if (!safeFileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        safeFileName += ".pdf";
    }

    QString filePath = dir.absoluteFilePath(safeFileName);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    qint64 bytesWritten = file.write(pdfData);
    file.close();

    return bytesWritten == pdfData.size();
}

// =========================== CONNECTION FUNCTIONS  ==============================  

void DiskNode::onReadyRead() {

    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    buffers[socket] += socket->readAll();

    while (buffers[socket].size() >= static_cast<qsizetype>(sizeof(quint32))) {
        QDataStream stream(buffers[socket]);
        quint32 messageLength;
        stream >> messageLength; 

        if (buffers[socket].size() - sizeof(quint32) >= messageLength) {
            QByteArray completeMessage = buffers[socket].mid(sizeof(quint32), messageLength);
            buffers[socket].remove(0, sizeof(quint32) + messageLength);

            messageFormat.readMessage(completeMessage);

            inputNotification(messageFormat, messageFormat.getFileName());
            if (messageFormat.getIndicator() == MessageIndicator::ControllerToNode)
            {
                QByteArray data;
                if (messageFormat.getAction() == ActionMessage::Upload){
                    bool success = storeFile(messageFormat.getContent(), messageFormat.getFileName());
                    qDebug().noquote() << QString("[Upload] Success       : %1").arg(success);
                    // if se puede subir data = "se puede subir" ---- else data = "No se puede subir"
                    data = "Se sube el pdf: " + messageFormat.getFileName().toUtf8();
                    MessageIndicator indicator = MessageIndicator::NodeToController;
                    ActionMessage action = ActionMessage::Upload;
                    QByteArray message = messageFormat.createFormat(indicator,messageFormat.getFileName(),action,data);
                    sendData(message);
                }

                else if (messageFormat.getAction() == ActionMessage::Erase)
                {
                    qDebug() << "Se intenta borrar un pdf: " + messageFormat.getFileName();
                    // logica de borrar un pdf......
                    // if se logra borrar ..........
                    data = "Se borra el pdf: " + messageFormat.getFileName().toUtf8();
                    MessageIndicator indicator = MessageIndicator::NodeToController;
                    ActionMessage action = ActionMessage::Erase;
                    QByteArray message = messageFormat.createFormat(indicator,messageFormat.getFileName(),action,data);
                    sendData(message);
                }

                else if (messageFormat.getAction() == ActionMessage::Download)
                {  
                    // Falta revisar como hacer la reconeccion de nodos.....
                    qDebug() << "El nodo empieza a mandar info para descarga de pdf";
                }

                else if (messageFormat.getAction() == ActionMessage::Check)
                {
                    qDebug() << "Se verifica estatus de pdf: " + messageFormat.getFileName();
                    // logica de borrar un pdf......
                    // if existe y se puede descargar..........
                    data = "El pdf esta disponible";
                    MessageIndicator indicator = MessageIndicator::NodeToController;
                    ActionMessage action = ActionMessage::Check;
                    QByteArray message = messageFormat.createFormat(indicator,messageFormat.getFileName(),action,data);
                    sendData(message);
                }
            }
            else
            {
                qDebug() << "Se recibio un mensaje que NO era para nodo!";
            }
            
        } else break;
    }
}

// =========================== SEND INFORMATION ==============================================

void DiskNode::sendData(const QByteArray &data) {

    if (!isConnected()) {
        qDebug() << "No conectado, no se puede enviar datos";
        return;
    }

    // Prepend message length (4 bytes) before sending
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << quint32(data.size()); // Write message size as a 4-byte header
    block.append(data); // Append the actual data

    qint64 bytesWritten = socket->write(block);
    if (bytesWritten == -1) {
        qDebug() << "Error al enviar datos:" << socket->errorString();
        return;
    }

    if (!socket->waitForBytesWritten(1000)) {
        qDebug() << "Timeout al enviar datos";
    }
}