#include "mainwindow.hpp"
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    // Inputs de texto
    input1 = new QLineEdit(this);
    input1->setPlaceholderText("Ingresa texto 1");
    input1->setGeometry(50, 20, 300, 30);

    input2 = new QLineEdit(this);
    input2->setPlaceholderText("Ingresa texto 2");
    input2->setGeometry(50, 60, 300, 30);

    // Botón para ir a ventana 2
    btnOpenSecond = new QPushButton("Ir a Ventana 2", this);
    btnOpenSecond->setGeometry(50, 110, 200, 50);

    secondWindow = new SecondWindow;

    connect(btnOpenSecond, &QPushButton::clicked, this, &MainWindow::openSecondWindow);
    connect(secondWindow, &SecondWindow::backToMain, this, &QWidget::show);

    this->setStyleSheet("background-color: lightblue;");

    QSize screenSize = qApp->primaryScreen()->availableGeometry().size();
    int w = screenSize.width() * 0.7;
    int h = screenSize.height() * 0.6;
    this->resize(w, h);
}

void MainWindow::openSecondWindow()
{
    secondWindow->show();
    this->hide();
}
