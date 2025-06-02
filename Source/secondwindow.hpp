#ifndef SECONDWINDOW_HPP
#define SECONDWINDOW_HPP

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

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
    void openPDFFile();
    void reconstructPDF();

private:
    void splitAndSavePDF(const QString &filePath);

    QPushButton *btnReturn;
    QPushButton *btnPrint;
    QPushButton *btnOpenPDF;
    QPushButton *btnReconstruct;
    QLineEdit *lineEditPDFName;
};

#endif // SECONDWINDOW_HPP
