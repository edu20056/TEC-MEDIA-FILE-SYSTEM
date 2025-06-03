#include "interface.hpp"

#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    btnPrint = new QPushButton("IMPRIMIR MENSAJE", this);
    btnPrint->setGeometry(50, 120, 200, 50);

    btnOpenPDF = new QPushButton("Seleccionar PDF", this);
    btnOpenPDF->setGeometry(50, 190, 200, 50);

    lineEditPDFName = new QLineEdit(this);
    lineEditPDFName->setGeometry(50, 260, 200, 30);
    lineEditPDFName->setPlaceholderText("Nombre PDF para reconstruir");

    btnReconstruct = new QPushButton("Reconstruir PDF", this);
    btnReconstruct->setGeometry(50, 300, 200, 50);

    connect(btnPrint, &QPushButton::clicked, this, &MainWindow::printMessage);
    connect(btnOpenPDF, &QPushButton::clicked, this, &MainWindow::openPDFFile);
    connect(btnReconstruct, &QPushButton::clicked, this, &MainWindow::reconstructPDF);

    this->setStyleSheet("background-color: lightcoral;");

    QSize screenSize = qApp->primaryScreen()->availableGeometry().size();
    int w = screenSize.width() * 0.7;
    int h = screenSize.height() * 0.6;
    this->resize(w, h);
}

void MainWindow::printMessage()
{
    qDebug() << "Botón presionado en Ventana 2: ¡Hola desde Qt!";
}

void MainWindow::openPDFFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Seleccionar archivo PDF", QDir::homePath(), "Archivos PDF (*.pdf)");
    if (!fileName.isEmpty()) {
        qDebug() << "Archivo PDF seleccionado:" << fileName;
        splitAndSavePDF(fileName);
    } else {
        qDebug() << "No se seleccionó ningún archivo.";
    }
}

void MainWindow::splitAndSavePDF(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No se pudo abrir el archivo PDF.";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

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


void MainWindow::reconstructPDF()
{
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
