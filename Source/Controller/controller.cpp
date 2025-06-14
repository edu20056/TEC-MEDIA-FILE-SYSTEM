#include "controller.hpp"buffclientNum

NodeController::NodeController(QObject *parent, quint16 port, quint64 block) : QTcpServer(parent)
{
    blockSize = block;
    qDebug() << "Inicialized as: " << port << ", Blocksize" << blockSize;
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Server could not start:" << errorString();
    } else {
        qInfo() << "Server listening on port" << port;
    }
}

// ======================= CONNECTION FUNCTIONS =====================================

void NodeController::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);
    
    connect(client, &QTcpSocket::readyRead, this, &NodeController::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &NodeController::onDisconnected);
    connect(client, &QTcpSocket::disconnected, client, &QTcpSocket::deleteLater);

    clients.append(client);
    ClientIdentificator identificator;
    identificator.id == 0;
    identificator.type == ClientType::Unknown;
    clientTypes.insert(client, identificator);  // <- nuevo
    qInfo() << "New client connected:" << client->peerAddress().toString();
}

void NodeController::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    // Acumulate data on buffer.
    buffers[client] += client->readAll();

    if (clientTypes[client].type == ClientType::Unknown) 
    {
        if (clientNum <= 4)  // First 4 incoming connections are nodes.
        {
            clientTypes[client].type = ClientType::DiskNode;
        }  
        else
        {
            clientTypes[client].type = ClientType::Gui;
        }
        clientTypes[client].id = clientNum;
        clientNum++;
    }
    // Procesar mensajes completos
    while (buffers[client].size() >= 4) { // We asume that the first 4 bytes are for message size
        // Read message size (first 4 bytes)
        quint32 messageLength;
        QDataStream stream(buffers[client]);
        stream >> messageLength;

        // Verify if message is complete
        if (buffers[client].size() - static_cast<qint64>(sizeof(quint32)) >= static_cast<qint64>(messageLength)) {
            // Extract message
            QByteArray completeMessage = buffers[client].mid(4, messageLength);
            buffers[client].remove(0, 4 + messageLength);

            // Message procesor
            messageFormat.readMessage(completeMessage);
            emit dataReceived(client, completeMessage);
            MessageIndicator indicator = messageFormat.getIndicator();
            qDebug() << "Received from client: "<< messageFormat.getFileName() << "Indicator message:" << messageFormat.messageIndicatorToString(indicator);
            qDebug() << "================================================================================";
            // Lecture for nodes logic begins here
            if (messageFormat.getAction() == ActionMessage::MemoryStatus)
            {
                break; // Wait for more data
            }
            if (messageFormat.getIndicator() == MessageIndicator::ServerToController) { // Incoming message from GUI
                if (messageFormat.getAction() == ActionMessage::Upload) {

                    

                    qDebug() << "Se intenta cargar el pdf: " + messageFormat.getFileName();
                    ActionMessage action = messageFormat.getAction();
                    QByteArray newMessage = messageFormat.createFormat(MessageIndicator::ControllerToNode,messageFormat.getFileName(),action, messageFormat.getContent());
                    for (QTcpSocket* nodo : clientTypes.keys()) {
                        if (clientTypes.value(nodo).type == ClientType::DiskNode) { 
                            sendData(nodo, newMessage);
                            qDebug() << "Enviado a nodo" << nodo->peerAddress().toString();
                            qDebug() << "================================================================================";
                        }
                    }            
                    // splitAndSave logic
                }
                else if (messageFormat.getAction() == ActionMessage::Erase)
                {
                    qDebug() << "Se intentara borrar: " + messageFormat.getFileName();
                    ActionMessage action = messageFormat.getAction();
                    QByteArray newMessage = messageFormat.createFormat(MessageIndicator::ControllerToNode,messageFormat.getFileName(),action, messageFormat.getContent());
                    for (QTcpSocket* nodo : clients) {
                        if (nodo != client) { // avoid to resend message 
                            sendData(nodo, newMessage);
                            qDebug() << "Enviado a nodo" << nodo->peerAddress().toString();
                            qDebug() << "================================================================================";
                        }
                    }  
                }
                else if (messageFormat.getAction() == ActionMessage::Check)
                {
                    qDebug() << "Revisa el estado de: " +  messageFormat.getFileName();
                    ActionMessage action = messageFormat.getAction();
                    QByteArray newMessage = messageFormat.createFormat(MessageIndicator::ControllerToNode,messageFormat.getFileName(),action, messageFormat.getContent());
                    for (QTcpSocket* nodo : clients) {
                        if (nodo != client) { // avoid to resend message 
                            sendData(nodo, newMessage);
                            qDebug() << "Enviado a nodo" << nodo->peerAddress().toString();
                            qDebug() << "================================================================================";
                        }
                    }  
                }
                else if (messageFormat.getAction() == ActionMessage::Download) 
                {
                    qDebug() << "Se intenta descargar: " +  messageFormat.getFileName();
                    ActionMessage action = messageFormat.getAction();
                    QByteArray newMessage = messageFormat.createFormat(MessageIndicator::ControllerToNode,messageFormat.getFileName(),action, messageFormat.getContent());
                    for (QTcpSocket* nodo : clients) {
                        if (nodo != client) { // avoid to resend message 
                            sendData(nodo, newMessage);
                            qDebug() << "Enviado a nodo" << nodo->peerAddress().toString();
                            qDebug() << "================================================================================";
                        }
                    }  
                }
                else
                {
                    qDebug() << "Ningun commando de action calsa :c";
                }
                
            }
            else if (messageFormat.getIndicator() == MessageIndicator::NodeToController) // Incoming from node
            {
                qDebug() << "Mensaje desde nodo: " + messageFormat.getContent();
                ActionMessage action = messageFormat.getAction();
                QByteArray data = messageFormat.getContent();
                QByteArray newMessage = messageFormat.createFormat(MessageIndicator::ControllerToServer, messageFormat.getFileName(), action, data);
                for (QTcpSocket* nodo : clientTypes.keys()) {
                    if (clientTypes.value(nodo).type == ClientType::Gui) {
                        sendData(nodo, newMessage);
                        qDebug() << "Enviado a nodo" << nodo->peerAddress().toString();
                        qDebug() << "================================================================================";
                    }
                } 

            }

        } else {
            break; // Wait for more data
        }
    }
}

void NodeController::onDisconnected(){

    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    clients.removeAll(client);
    qInfo() << "Client disconnected:" << client->peerAddress().toString();
}

void NodeController::uploadBlksIntoNodes(const QByteArray& fileData, const QString& fileName, quint64 blockSize) {

    QList<QByteArray> blocks = splitIntoBlocks(fileData, blockSize);
    QList<QTcpSocket*> nodeSockets;

    for (QTcpSocket* socket : clientTypes.keys()) {
        if (clientTypes[socket].type == ClientType::DiskNode) {
            nodeSockets.append(socket);
        }
    }

    if (nodeSockets.size() < 4) {
        qWarning() << "!ERROR: NOT ENOUGH DISKS";
        return;
    }

    int rowsNum = blocks.size() / 3;
    int globalBlockIndex = 0;

    for (int g = 0; g < rowsNum; ++g) {
        QList<QByteArray> packages;

        // Extract the 3 blocks corresponding to the iteration
        for (int j = 0; j < 3; ++j) {

        }

    }
}

void NodeController::sendData(QTcpSocket *client, const QByteArray &data) {
    if (!client || !client->isWritable()) {
        qDebug() << "Error: Socket no válido o no escribible";
        return;
    }

    // Creates a new longitude prefix (4 bytes)
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << quint32(data.size());  // Longitude Prefix
    block.append(data);           // Real data 

    // Send block
    qint64 bytesWritten = client->write(block);
    if (bytesWritten == -1) {
        qDebug() << "Error al enviar datos:" << client->errorString();
        return;
    }

    if (!client->waitForBytesWritten(1000)) {
        qDebug() << "Timeout al enviar datos";
    } else {
        qDebug() << "Datos enviados correctamente. Tamaño:" << block.size();
    }
}

// ========================= PDF SYSTEM FUNCTIONS =========================================

QList<QByteArray> NodeController::splitIntoBlocks(const QByteArray& data, quint64 blockSize) {

    QList<QByteArray> blocks;
    int totalSize = data.size();
    int offset = 0;

    while (offset < totalSize) {
        QByteArray block = data.mid(offset, blockSize);

        if (block.size() < blockSize) {
            block.append(QByteArray(blockSize - block.size(), '\0'));
        }

        blocks.append(block);
        offset += blockSize;
    }

    int remainder = blocks.size() % 3;
    if (remainder != 0) {
        int blocksToAdd = 3 - remainder;
        for (int i = 0; i < blocksToAdd; ++i) {
            blocks.append(QByteArray(blockSize, '\0'));
        }
    }

    return blocks;
}

QByteArray NodeController::calculateParity(const QByteArray& block1, const QByteArray& block2, const QByteArray& block3) {

    if (!(block1.size() == block2.size() && block2.size() == block3.size())) {
        qWarning() << "!ERROR : Something wrong with block size";
        return QByteArray();
    }

    int blockSize = block1.size();
    QByteArray parityBlock(blockSize, '\0');

    for (int i = 0; i < blockSize; ++i) {
        parityBlock[i] = block1[i] ^ block2[i] ^ block3[i];
    }

    return parityBlock;
}

void NodeController::reconstructPDF(QString pdfName) { // pdfName must be obtained by a httpFormat object

    if (pdfName.isEmpty()) {
        qDebug() << "Debes ingresar un nombre de PDF.";
        return;
    }

    QString saveDir = QDir::currentPath() + "/Node";
    QString finalPDFPath = saveDir + "/" + pdfName + "_RECUPERADO.pdf";

    QFile finalPDF(finalPDFPath);
    if (!finalPDF.open(QIODevice::WriteOnly)) {
        qDebug() << "No se pudo crear el PDF recuperado.";
        return;
    }

    for (int i = 0; i < 3; ++i) {
        QString partFileName = saveDir + "/" + pdfName + "_" + QString::number(i + 1);
        QFile partFile(partFileName);
        if (partFile.open(QIODevice::ReadOnly)) {
            QByteArray partData = partFile.readAll();
            finalPDF.write(partData);
            partFile.close();

            qDebug() << "Parte" << i + 1 << "agregada:" << partFileName << "(" << partData.size() << "bytes )";
        } else {
            qDebug() << "No se pudo abrir" << partFileName;
            finalPDF.close();
            return;
        }
    }

    finalPDF.close();
    qDebug() << "PDF reconstruido en:" << finalPDFPath;
}