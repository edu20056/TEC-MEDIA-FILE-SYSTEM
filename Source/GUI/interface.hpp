#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

signals:

private slots:
    void printMessage();
    void openPDFFile();
    void reconstructPDF();

private:
    void splitAndSavePDF(const QString &filePath);

    QPushButton *btnPrint;
    QPushButton *btnOpenPDF;
    QPushButton *btnReconstruct;
    QLineEdit *lineEditPDFName;
};

#endif
