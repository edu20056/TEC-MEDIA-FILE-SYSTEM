#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <QWidget>
#include <QTcpSocket>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

class App : public QWidget {
    Q_OBJECT

public:
    App(QWidget *parent = nullptr, const QString &host = "localhost", quint16 port = 50000);

    void sendData(const QByteArray &data);
    bool isConnected() const;

signals:
    void connectionStatusChanged(bool connected);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    
    void printMessage();
    void openPDFFile();
    void reconstructPDF();

private:
    QTcpSocket *socket;
    void splitAndSavePDF(const QString &filePath);

    QPushButton *btnPrint;
    QPushButton *btnOpenPDF;
    QPushButton *btnReconstruct;
    QLineEdit *lineEditPDFName;
};

#endif
