#include "controller.hpp"

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
    identificator.id = 0;
    identificator.type = ClientType::Unknown;
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
                    
                    uploadBlksIntoNodes(messageFormat.getContent(), messageFormat.getFileName(), blockSize);
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
                
                if (messageFormat.getAction() == ActionMessage::Download) 
                {
                    if (messageFormat.getContent().contains("FINISHED"))
                    {
                        currentNodeLoaded++;
                        qDebug() << "NODO LISTO!";
                        if (currentNodeLoaded == 4)
                        {
                            qDebug() << "PDF LISTO PARA RECONSTRUIR";
                            reconstructPDF(messageFormat.getFileName());
                            currentNodeLoaded = 0;
                            incomingDataToDownload.clear();
                        }
                    }
                    else
                    {
                        incomingDataToDownload.insert(messageFormat.getFileName(), messageFormat.getContent());
                    }

                }
                
                else // Check, Delete, AND Upload show generic answer message.
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

    QMap<int, QTcpSocket*> idToSocket;
    for (QTcpSocket* socket : clientTypes.keys()) {
        if (clientTypes[socket].type == ClientType::DiskNode) {
            idToSocket[clientTypes[socket].id] = socket;
        }
    }

    QList<QTcpSocket*> nodeSockets = idToSocket.values();
    int numDisks = nodeSockets.size();

    if (numDisks < 4) {
        qWarning() << "!ERROR: NOT ENOUGH DISKS";
        return;
    }

    int groupSize = numDisks - 1;
    int totalGroups = blocks.size() / groupSize;
    int dataBlockIndex = 0;
    int parityCount = 1;

    for (int g = 0; g < totalGroups; ++g) {
        QList<QByteArray> dataBlocks;
        
        for (int i = 0; i < groupSize; ++i) {
            dataBlocks.append(blocks[g * groupSize + i]);
        }

        QByteArray parityBlock = calculateParity(dataBlocks[0], dataBlocks[1], dataBlocks[2]);

        int parityIndex = (numDisks - 1 - ((currentRaidRow + g) % numDisks));
        int dataIndex = 0;

        for (int d = 0; d < numDisks; ++d) {
            QByteArray blockToSend;
            QString blockName;

            if (d == parityIndex) {
                blockToSend = parityBlock;
                blockName = QString("%1_p%2").arg(fileName).arg(parityCount++);
            } else {
                blockToSend = dataBlocks[dataIndex++];
                blockName = QString("%1_%2").arg(fileName).arg(dataBlockIndex++);
            }

            ActionMessage action = ActionMessage::Upload;
            QByteArray formattedMessage = messageFormat.createFormat(
                MessageIndicator::ControllerToNode,
                blockName,
                action,
                blockToSend
            );

            sendData(nodeSockets[d], formattedMessage);
            qDebug() << "BLK" << blockName << "SENT TO NODE ID" << (d + 1);
        }
    }

    currentRaidRow += totalGroups;
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
    quint64 totalSize = data.size();
    quint64 offset = 0;

    while (offset < totalSize) {
        QByteArray block = data.mid(offset, blockSize);

        if (static_cast<quint64>(block.size()) < blockSize) {
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

void NodeController::reconstructPDF(QString pdfName) { 

    if (pdfName.isEmpty()) {
        qDebug() << "Debes ingresar un nombre de PDF.";
        return;
    }

    if (incomingDataToDownload.isEmpty()) {
        qDebug() << "No se encontraron bloques de datos para reconstruir en mapa OG";
        return;
    }

    QMap<int, QByteArray> bloquesOrdenados;

    QString pattern = QString("^%1_(\\d+)$").arg(QRegularExpression::escape(pdfName));
    QRegularExpression regex(pattern);

    for (auto it = incomingDataToDownload.begin(); it != incomingDataToDownload.end(); ++it) {
        QRegularExpressionMatch match = regex.match(it.key());
        if (match.hasMatch()) {
            int numero = match.captured(1).toInt();
            bloquesOrdenados[numero] = it.value();
        }
    }

    if (bloquesOrdenados.isEmpty()) {
        qDebug() << "No se encontraron bloques de datos para reconstruir.";
        return;
    }

    // Connect Blocks in order
    QByteArray pdfCompleto;
    for (auto it = bloquesOrdenados.begin(); it != bloquesOrdenados.end(); ++it) {
        pdfCompleto.append(it.value());
    }

    QString rutaSalida = "../Pdf/Download/" + pdfName + ".pdf";


    QFile archivo(rutaSalida);
    if (archivo.open(QIODevice::WriteOnly)) {
        archivo.write(pdfCompleto);
        archivo.close();
        qDebug() << "Archivo PDF reconstruido correctamente en:" << rutaSalida;
    } else {
        qDebug() << "No se pudo guardar el archivo PDF.";
    }
}