#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include "secondwindow.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void openSecondWindow();

private:
    QPushButton *btnOpenSecond;
    QLineEdit *input1;
    QLineEdit *input2;
    SecondWindow *secondWindow;
};

#endif // MAINWINDOW_HPP
