#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QByteArray>

class NodeController : public QTcpServer {
    Q_OBJECT
public:
    explicit NodeController(QObject *parent = nullptr, quint16 port = 50000);
    void sendData(QTcpSocket *client, const QByteArray &data);

signals:
    void dataReceived(QTcpSocket *client, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    QList<QTcpSocket*> clients;
};

#endif