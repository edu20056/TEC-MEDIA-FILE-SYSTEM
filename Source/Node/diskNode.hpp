#ifndef DISKNODE_HPP
#define DISKNODE_HPP

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QTcpSocket>
#include <QDir>
#include <QStringList>
#include <QDataStream>
#include "../HTTP/httpFormat.hpp"
#include "../Config/xmlReader.hpp"

class DiskNode : public QObject {
    Q_OBJECT

public:
    explicit DiskNode(QObject *parent = nullptr,
        const QString &host = "localhost",
        quint16 port = 50000,
        const QString path = "",
        quint16 id = 0, 
        quint64 blk = 0, 
        quint64 disk = 0
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

    QByteArray buildMessage(MessageIndicator indicator, const QString &fileName,
        ActionMessage action, const QByteArray &data);
    void sendStatus();

    void nodeInfo() const;
    void inputNotification(httpFormat const &messageData, QString fileName) const;

    bool initPath();
    bool storeFile(const QByteArray& data, QString fileName);
    bool reconstructPdf(const QByteArray& pdfData, const QString& fileName);
    void deleteFile(QString const &fileName); 
    void searchAndSendPdfBlocks(const QString& path, const QString& fileName );

private:
    QTcpSocket *socket;
    QString path;
    quint16 nodeID;
    quint64 blkSize; 
    quint64 diskSize; 
    httpFormat messageFormat;
    QHash<QTcpSocket*, QByteArray> buffers;
    QStringList fileNamesAdded;
};

#endif