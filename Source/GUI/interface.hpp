#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <QWidget>
#include <QTcpSocket>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QFile>
#include <QDir>
#include "../HTTP/httpFormat.hpp"

class App : public QWidget {
    Q_OBJECT

public:
    App(QWidget *parent = nullptr, const QString &host = "localhost", quint16 port = 50000);

    void sendData(const QByteArray &data);
    bool isConnected() const;
    void updateNodeStatus(int nodeID, const QStringList &fileList);

signals:
    void connectionStatusChanged(bool connected);

private slots:

    void onReadyRead();
    void erasePDF(); // erase
    void UploadPDF(); // upload
    void Download(); // download
    void CheckExistent(); // check
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

    int extractNodeID(const QString &status); 
    QStringList extractFileNames(const QString &status); 

private:
    httpFormat messageFormat;
    QTcpSocket *socket;
    QPushButton *btnErase;
    QPushButton *btnCheck; 
    QPushButton *btnDownload;
    QPushButton *btnUpload;
    QLineEdit *lineEditPDFName;
    QHash<QTcpSocket*, QByteArray> buffers; // To receive messages

    QTableWidget *nodeStatusTable;
    void setupNodeStatusTable(); 
};

#endif
