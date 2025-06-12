#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include "../HTTP/httpFormat.hpp"
#include "../Config/xmlReader.hpp"

class NodeController : public QTcpServer {
    Q_OBJECT
public:
    int blockSize;
    explicit NodeController(QObject *parent = nullptr, quint16 port = 50000, quint64 blockSize = 1024);
    void sendData(QTcpSocket *client, const QByteArray &data);
    void splitAndSavePDF();
    void reconstructPDF(QString pdfName);

signals:
    void dataReceived(QTcpSocket *client, QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:    
    QList<QTcpSocket*> clients;
    httpFormat messageFormat;
    QHash<QTcpSocket*, QByteArray> buffers;
};

#endif