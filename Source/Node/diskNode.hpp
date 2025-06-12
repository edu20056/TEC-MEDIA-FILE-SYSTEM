#ifndef DISKNODE_HPP
#define DISKNODE_HPP

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QTcpSocket>
#include <QDir>
#include "../HTTP/httpFormat.hpp"
#include "../Config/xmlReader.hpp"

class DiskNode : public QObject {
    Q_OBJECT

public:
    explicit DiskNode(QObject *parent = nullptr,
        const QString &host = "localhost",
        quint16 port = 50000,
        const QString path = "",
        quint16 = 0
    );

    void sendData(const QByteArray &data);
    bool isConnected() const;

signals:
    void connectionStatusChanged(bool connected);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

    void nodeInfo() const;
    bool initPath();

private:
    QTcpSocket *socket;
    QString path;
    quint16 nodeID;
    httpFormat messageFormat;
};

#endif