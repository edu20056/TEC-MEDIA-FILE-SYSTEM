#ifndef DISKNODE_HPP
#define DISKNODE_HPP

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QTcpSocket>
#include "../HTTP/httpFormat.hpp"
#include "../Config/xmlReader.hpp"

class DiskNode : public QObject {
    Q_OBJECT

public:
    explicit DiskNode(QObject *parent = nullptr, const QString &host = "localhost", quint16 port = 50000);

    void sendData(const QByteArray &data);
    bool isConnected() const;

signals:
    void connectionStatusChanged(bool connected);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *socket;
    httpFormat messageFormat;
};

#endif