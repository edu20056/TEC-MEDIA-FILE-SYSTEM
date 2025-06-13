#include "interface.hpp"

App::App(QWidget *parent, const QString &host, quint16 port) : QWidget(parent), socket(new QTcpSocket(this)) {

    connect(socket, &QTcpSocket::readyRead, this, &App::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &App::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &App::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &App::onError);

    socket->connectToHost(host, port);

    // Erase and Upload
    btnErase = new QPushButton("Borrar PDF", this);
    btnErase->setGeometry(50, 120, 200, 50);

    btnUpload = new QPushButton("Subir (upload) PDF", this);
    btnUpload->setGeometry(300, 60, 200, 50);

    // Open PDF
    btnDownload = new QPushButton("Descargar PDF", this);
    btnDownload->setGeometry(50, 60, 200, 50);

    // Check PDF
    btnCheck = new QPushButton("Revisar existencia de PDF", this);
    btnCheck->setGeometry(50, 190, 200, 50);

    lineEditPDFName = new QLineEdit(this);
    lineEditPDFName->setGeometry(50, 260, 200, 30);
    lineEditPDFName->setPlaceholderText("Nombre PDF para reconstruir");

    connect(btnErase, &QPushButton::clicked, this, &App::erasePDF); // Erase
    connect(btnUpload, &QPushButton::clicked, this, &App::UploadPDF); // Upload pdf
    connect(btnDownload, &QPushButton::clicked, this, &App::Download); // Download
    connect(btnCheck, &QPushButton::clicked, this, &App::CheckExistent); // Download
    this->setStyleSheet("background-color: lightcoral;");

    QSize screenSize = qApp->primaryScreen()->availableGeometry().size();
    int w = screenSize.width() * 0.7;
    int h = screenSize.height() * 0.6;
    this->resize(w, h);
}

// ======================== CONNECTION FUNCTIONS ============================================

void App::onConnected() {

    qDebug() << "Conectado al servidor";
    emit connectionStatusChanged(true);
}

void App::onDisconnected() {

    qDebug() << "Desconectado del servidor";
    emit connectionStatusChanged(false);
}

void App::onError(QAbstractSocket::SocketError error) {

    Q_UNUSED(error);
    qDebug() << "Error de socket:" << socket->errorString();
}


bool App::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

void App::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    // Acumulate data on buffer.
    buffers[client] += client->readAll();

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
            if (messageFormat.getIndicator() == MessageIndicator::ControllerToServer)
            {
                if (messageFormat.getAction() == ActionMessage::Upload)
                {
                    qDebug() << "Se intento subir el pdf: " + messageFormat.getFileName() + ". Se obtuvo como resultado: " + messageFormat.getContent();
                    // resultado debe decir "Pdf no se logro subir" o "Pdf subido exitosamente"
                }

                else if (messageFormat.getAction() == ActionMessage::Erase)
                {
                    qDebug() << "Se intento borrar el pdf: " + messageFormat.getFileName() + ". Se obtuvo como resultado: " + messageFormat.getContent();
                    // resultado debe decir "Pdf no existia" o "Pdf borrado exitosamente"
                }

                else if (messageFormat.getAction() == ActionMessage::Download)
                {
                    qDebug() << "Se descarga el file: " + messageFormat.getFileName() + ". En la direccion: " + messageFormat.getContent();
                    // En teoria el content deberia ser la direccion donde se descargo el pdf
                }

                else if (messageFormat.getAction() == ActionMessage::Check)
                {
                    qDebug() << "Se checkeo el estado de: " + messageFormat.getFileName() + ". Estado: " + messageFormat.getContent();
                    // Content deberia decir si esta disponible el nombre buscado.
                }
            }

            else
            {
                qDebug() << "Se recibio un mensaje no deseado para GUI";
            }
        }
    }
}

// =========================== SEND INFORMATION WITH BUTTONS =================================

void App::sendData(const QByteArray &data) {
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
void App::erasePDF()
{
    QString pdfName = lineEditPDFName->text();
    if (pdfName.isEmpty()) {
        qDebug() << "Debes ingresar un nombre de PDF.";
        return;
    }
    
    ActionMessage action = ActionMessage::Erase;   
    QByteArray _ = "";
    QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController,pdfName, action, _);
    sendData(message);
}

void App::UploadPDF() {
    QString fileName = QFileDialog::getOpenFileName(this, "Seleccionar archivo PDF", QDir::homePath(), "Archivos PDF (*.pdf)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    QFileInfo fileIndo(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No se pudo abrir el archivo:" << fileName;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        qDebug() << "El archivo PDF está vacío:"  << fileName;
        return;
    }

    QString baseName = fileIndo.completeBaseName();
    ActionMessage action = ActionMessage::Upload; 
    QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController, baseName, action, data);
    sendData(message);
}


void App::Download() {

    QString fileName = lineEditPDFName->text(); 

    if (!fileName.isEmpty()) {
        qDebug() << "Archivo PDF seleccionado:" << fileName;

        ActionMessage action = ActionMessage::Download; 
        QByteArray _ = "";
        QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController,fileName, action, _);
        sendData(message);
    } else {
        qDebug() << "No se seleccionó ningún archivo." << fileName;
    }
}

void App::CheckExistent() {

    QString fileName = lineEditPDFName->text(); 

    if (!fileName.isEmpty()) {
        qDebug() << "Archivo PDF seleccionado:" << fileName;

        ActionMessage action = ActionMessage::Check; 
        QByteArray _ = "";
        QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController,fileName, action, _);
        sendData(message);
    } else {
        qDebug() << "No se seleccionó ningún archivo." << fileName;
    }
}