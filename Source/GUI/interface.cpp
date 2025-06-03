#include "interface.hpp"

#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QFile>
#include <QDir>

App::App(QWidget *parent, const QString &host, quint16 port) : QWidget(parent), socket(new QTcpSocket(this)) {

    connect(socket, &QTcpSocket::connected, this, &App::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &App::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &App::onError);

    socket->connectToHost(host, port);

    btnPrint = new QPushButton("IMPRIMIR MENSAJE", this);
    btnPrint->setGeometry(50, 120, 200, 50);

    btnOpenPDF = new QPushButton("Seleccionar PDF", this);
    btnOpenPDF->setGeometry(50, 190, 200, 50);

    lineEditPDFName = new QLineEdit(this);
    lineEditPDFName->setGeometry(50, 260, 200, 30);
    lineEditPDFName->setPlaceholderText("Nombre PDF para reconstruir");

    btnReconstruct = new QPushButton("Reconstruir PDF", this);
    btnReconstruct->setGeometry(50, 300, 200, 50);

    connect(btnPrint, &QPushButton::clicked, this, &App::printMessage);
    connect(btnOpenPDF, &QPushButton::clicked, this, &App::openPDFFile);
    connect(btnReconstruct, &QPushButton::clicked, this, &App::reconstructPDF);

    this->setStyleSheet("background-color: lightcoral;");

    QSize screenSize = qApp->primaryScreen()->availableGeometry().size();
    int w = screenSize.width() * 0.7;
    int h = screenSize.height() * 0.6;
    this->resize(w, h);
}

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

void App::sendData(const QByteArray &data) {

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

bool App::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

void App::printMessage() {

    qDebug() << "Botón presionado en Ventana 2: ¡Hola desde Qt!";
}

void App::openPDFFile() {

    QString fileName = QFileDialog::getOpenFileName(this, "Seleccionar archivo PDF", QDir::homePath(), "Archivos PDF (*.pdf)");
    if (!fileName.isEmpty()) {
        qDebug() << "Archivo PDF seleccionado:" << fileName;
        splitAndSavePDF(fileName);
    } else {
        qDebug() << "No se seleccionó ningún archivo.";
    }
}

void App::splitAndSavePDF(const QString &filePath) {

    QFile file(filePath);
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

    sendData(data);

    qint64 totalSize = data.size();
    qint64 partSize = totalSize / 3;
    qint64 remainder = totalSize % 3;

    QString baseName = QFileInfo(filePath).baseName();
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

void App::reconstructPDF() {

    QString pdfName = lineEditPDFName->text();
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
