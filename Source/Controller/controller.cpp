#include "controller.hpp"

NodeController::NodeController(QObject *parent, quint16 port, quint64 block) : QTcpServer(parent), messageFormat(3)
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
    messageFormat.readMessage(data);
    emit dataReceived(client, data);
    qDebug() << "Received from client:" << data << "Indicator message:" << messageFormat.getIndicator();
    if (messageFormat.getIndicator() == 1) { // Incoming message from GUI
        if (messageFormat.getAction() == "upload")
        {
            qDebug() << "Se cargo pdf :D";
            // splitAndSave
        }
        else if (messageFormat.getAction() == "erase")
        {
            qDebug() << "Se borro un pdf";
            // for nodo in nodos, enviar mensaje de borrar con messageFormat.getFileName()
        }
        else if (messageFormat.getAction() == "check")
        {
            qDebug() << "Se hizo check de existencia de pdf";
            // for pdf in pdfList, revisar si existe alguno subido con messageFormat.getFileName()
        }
        else if (messageFormat.getAction() == "download") 
        {
            qDebug() << "Se descarga un pdf";
            // for nodo in nodos, relizar reconstruccion de pdf, TODO: ver donde se debe guardar
        }
        else
        {
            qDebug() << "Ningun commando de action calsa :c";
        }
        
    }
    else if (messageFormat.getIndicator() == 4) // Incoming from node
    {
        qDebug() << "Mensaje de nodo";
    }
    
}

void NodeController::onDisconnected(){

    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    clients.removeAll(client);
    qInfo() << "Client disconnected:" << client->peerAddress().toString();
}

// ========================= PDF SYSTEM FUNCTIONS =========================================

void NodeController::sendData(QTcpSocket *client, const QByteArray &data){

    if (client && client->state() == QAbstractSocket::ConnectedState) {
        client->write(data);
        client->waitForBytesWritten(1000);
    }
}

void NodeController::splitAndSavePDF(){
    QFile file("Source");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No se pudo abrir el archivo PDF.";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        qDebug() << "El archivo PDF está vacío";
        return;
    }

    qint64 totalSize = data.size();
    qint64 partSize = totalSize / 3;
    qint64 remainder = totalSize % 3;

    QString baseName = QFileInfo("Source").baseName();
    QString saveDir = QDir::currentPath() + "/Node";

    if (!QDir(saveDir).exists()) {
        QDir().mkdir(saveDir);
    }

    qint64 position = 0;
    for (int i = 0; i < 3; ++i) {
        QString partFileName = saveDir + "/" + baseName + "_" + QString::number(i + 1);
        QFile partFile(partFileName);
        if (partFile.open(QIODevice::WriteOnly)) {
            qint64 bytesToWrite = partSize;
            if (i == 2) bytesToWrite += remainder;

            partFile.write(data.constData() + position, bytesToWrite);
            partFile.close();

            qDebug() << "Parte" << i + 1 << "guardada:" << partFileName << "(" << bytesToWrite << "bytes )";

            position += bytesToWrite;
        } else {
            qDebug() << "No se pudo guardar la parte" << i + 1;
        }
    }
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