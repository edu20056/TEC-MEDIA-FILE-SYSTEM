#include "interface.hpp"

App::App(QWidget *parent, const QString &host, quint16 port) : QWidget(parent), messageFormat(1), socket(new QTcpSocket(this)) {

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
    QByteArray message = messageFormat.createFormat(1,pdfName, action, _);
    sendData(message);
}

void App::UploadPDF() {
    QString fileName = QFileDialog::getOpenFileName(this, "Seleccionar archivo PDF", QDir::homePath(), "Archivos PDF (*.pdf)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
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

    ActionMessage action = ActionMessage::Upload; 
    QByteArray message = messageFormat.createFormat(1, fileName, action, data);
    sendData(message);
}


void App::Download() {

    QString fileName = lineEditPDFName->text(); 

    if (!fileName.isEmpty()) {
        qDebug() << "Archivo PDF seleccionado:" << fileName;

        ActionMessage action = ActionMessage::Download; 
        QByteArray _ = "";
        QByteArray message = messageFormat.createFormat(1,fileName, action, _);
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
        QByteArray message = messageFormat.createFormat(1,fileName, action, _);
        sendData(message);
    } else {
        qDebug() << "No se seleccionó ningún archivo." << fileName;
    }
}