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
#include <QRegularExpression>
#include <QDataStream>
#include "../HTTP/httpFormat.hpp"
#include "../Config/xmlReader.hpp"

enum class ClientType {
    Unknown,
    Gui,
    DiskNode
};

struct ClientIdentificator
{
    ClientType type;
    int id; // From 1 to 4 are nodes, 5 is the GUI
};


class NodeController : public QTcpServer {
    Q_OBJECT
public:
    int blockSize;
    explicit NodeController(QObject *parent = nullptr, quint16 port = 50000, quint64 blockSize = 1024);
    void sendData(QTcpSocket *client, const QByteArray &data);
    QList<QByteArray> splitIntoBlocks(const QByteArray& data, quint64 blockSize);
    QByteArray calculateParity(const QByteArray& block1, const QByteArray& block2, const QByteArray& block3);
    void uploadBlksIntoNodes(const QByteArray& fileData, const QString& fileName, quint64 blockSize);
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
    int clientNum = 1;
    QHash<QTcpSocket*, ClientIdentificator> clientTypes;
    int currentRaidRow = 0;
    QMap<QString, QByteArray> incomingDataToDownload;
    int currentNodeLoaded = 0;
};

#endif