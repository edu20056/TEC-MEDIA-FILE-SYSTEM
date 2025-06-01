#ifndef SECONDWINDOW_HPP
#define SECONDWINDOW_HPP

#include <QWidget>
#include <QPushButton>

class SecondWindow : public QWidget
{
    Q_OBJECT

public:
    SecondWindow(QWidget *parent = nullptr);

signals:
    void backToMain();

private slots:
    void returnToMain();
    void printMessage();

private:
    QPushButton *btnReturn;
    QPushButton *btnPrint;
};

#endif // SECONDWINDOW_HPP
