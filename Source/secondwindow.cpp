#include "secondwindow.hpp"
#include <QApplication>
#include <QScreen>
#include <QDebug>

SecondWindow::SecondWindow(QWidget *parent) : QWidget(parent)
{
    btnReturn = new QPushButton("Volver a Ventana 1", this);
    btnReturn->setGeometry(50, 50, 200, 50);

    btnPrint = new QPushButton("Imprimir mensaje", this);
    btnPrint->setGeometry(50, 120, 200, 50);

    connect(btnReturn, &QPushButton::clicked, this, &SecondWindow::returnToMain);
    connect(btnPrint, &QPushButton::clicked, this, &SecondWindow::printMessage);

    this->setStyleSheet("background-color: lightcoral;");

    QSize screenSize = qApp->primaryScreen()->availableGeometry().size();
    int w = screenSize.width() * 0.7;
    int h = screenSize.height() * 0.6;
    this->resize(w, h);
}

void SecondWindow::returnToMain()
{
    emit backToMain();
    this->hide();
}

void SecondWindow::printMessage()
{
    qDebug() << "Botón presionado en Ventana 2: ¡Hola desde Qt!";
}
