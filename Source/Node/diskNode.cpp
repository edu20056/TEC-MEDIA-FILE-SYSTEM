#include "diskNode.hpp"

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
    sendStatus();
    emit connectionStatusChanged(true);
    nodeInfo(); 
}

void DiskNode::onDisconnected() {
    sendStatus();
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
    sendStatus();
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

    sendStatus();
    sendStatus();
    return true; 
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

void DiskNode::deleteFile(QString const &fileName) {

    QDir dir(path);

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    for (const QString &baseName : files) {
        QString prefix = baseName.section('_', 0, 0);

        if (prefix == fileName) {
            QString filePath = dir.absoluteFilePath(baseName);
            QFile file(filePath);
            file.remove();
        }
    }

    sendStatus();
}

// ================================= AUXILIARY =====================================  

QByteArray DiskNode::buildMessage(MessageIndicator indicator, const QString &fileName,
                                  ActionMessage action, const QByteArray &data) {
    return messageFormat.createFormat(indicator, fileName, action, data);
}

void DiskNode::searchAndSendPdfBlocks(const QString& path, const QString& fileName){
    QDir dir(path);

    if (!dir.exists()) {
        qDebug() << "La carpeta no existe.";
        return;
    }

    // List with file names on path
    QStringList archivos = dir.entryList(QDir::Files);

    for (const QString& archivo : archivos) {
        if (archivo.startsWith(fileName)) {

            // Open and read file
            QFile file(dir.filePath(archivo));
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray content = file.readAll();
                file.close();
                sendData(buildMessage(MessageIndicator::NodeToController,archivo, ActionMessage::Download,content));
                
            } else {
                qDebug() << "No se pudo abrir el archivo:" << archivo;
            }
            
        }
    }
    sendData(buildMessage(MessageIndicator::NodeToController,fileName, ActionMessage::Download,"FINISHED"));
                
}

// =========================== CONNECTION FUNCTIONS  ==============================  

void DiskNode::sendStatus() {
    if (!isConnected()) {
        qDebug() << "No conectado, no se puede enviar estatus.";
        return;
    }

    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "Directorio del nodo no existe.";
        return;
    }

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList fileNames;

    for (const QString &file : files) {
        fileNames << file;
    }

    QString statusText;
    statusText += QString("Node ID: %1\n").arg(nodeID);
    statusText += QString("Connected: %1\n").arg(isConnected() ? "Yes" : "No");
    statusText += QString("File Count: %1\n").arg(fileNames.size());

    for (const QString &file : fileNames) {
        statusText += QString(" - %1\n").arg(file);
    }

    QByteArray data = statusText.toUtf8();
    QByteArray message = buildMessage(
        MessageIndicator::NodeToController,
        "StatusReport",
        ActionMessage::MemoryStatus,
        data
    );

    sendData(message);
}

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

            if (messageFormat.getIndicator() == MessageIndicator::ControllerToNode) {
                QByteArray data;

                const QString &fileName = messageFormat.getFileName();

                if (messageFormat.getAction() == ActionMessage::Upload) {

                    if (!fileNamesAdded.contains(messageFormat.getFileName()))
                    {
                        fileNamesAdded.append(messageFormat.getFileName());

                    }

                    bool success = storeFile(messageFormat.getContent(), fileName);
                    qDebug().noquote() << QString("[Upload] Success       : %1").arg(success);

                    data = "Se sube el pdf: " + fileName.toUtf8();
                    sendData(buildMessage(MessageIndicator::NodeToController, fileName, ActionMessage::Upload, data));
                }

                else if (messageFormat.getAction() == ActionMessage::Erase) {

                    deleteFile(fileName); 

                    data = "Se borra el pdf: " + fileName.toUtf8();
                    sendData(buildMessage(MessageIndicator::NodeToController, fileName, ActionMessage::Erase, data));
                }

                else if (messageFormat.getAction() == ActionMessage::Download) {
                    qDebug() << "El nodo empieza a mandar info para descarga de pdf";
                    searchAndSendPdfBlocks(path,messageFormat.getFileName());
                }

                else if (messageFormat.getAction() == ActionMessage::Check) {
                    qDebug() << "Se verifica estatus de pdf: " + fileName;

                    if (!fileNamesAdded.contains(fileName))
                    {
                        data = "El pdf no se encuentra disponible.";
                        qDebug() << "No disponible";
                    }
                    else
                    {
                        data = "El pdf está disponible";
                    }

                    qDebug() << "Pdf disponible";
                    sendData(buildMessage(MessageIndicator::NodeToController, fileName, ActionMessage::Check, data));
                }

            } else {
                qDebug() << "Se recibió un mensaje que NO era para nodo!";
            }

        } else break;
    }
}

void DiskNode::sendData(const QByteArray &data) {

    if (!isConnected()) {
        qDebug() << "No conectado, no se puede enviar datos";
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << quint32(data.size());
    block.append(data);

    qint64 bytesWritten = socket->write(block);
    if (bytesWritten == -1) {
        qDebug() << "Error al enviar datos:" << socket->errorString();
        return;
    }

    if (!socket->waitForBytesWritten(1000)) {
        qDebug() << "Timeout al enviar datos";
    }
}