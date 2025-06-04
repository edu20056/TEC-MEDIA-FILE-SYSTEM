#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include <QTcpSocket>

// This will be use in the controller to read and set up the disk-nodes
// Alternately each Node will be compile in a 'main' individually...
void configRead(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "ERROR: XML couldn't be opened..." << filePath;
        return;
    }

    QXmlStreamReader xml(&file);
    QString currentTag;
    QMap<QString, QString> configValues;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement) {
            currentTag = xml.name().toString();
        } else if (token == QXmlStreamReader::Characters && !currentTag.isEmpty()) {
            configValues[currentTag] = xml.text().toString();
            currentTag.clear();
        }
    }

    if (xml.hasError()) {
        qDebug() << "ERROR: while reading XML..." << xml.errorString();
    }

    file.close();

    qDebug() << "NodeID:" << configValues.value("NodeID");
    qDebug() << "ControllerIP:" << configValues.value("ControllerIP");
    qDebug() << "ControllerPort:" << configValues.value("ControllerPort");
    qDebug() << "StoragePath:" << configValues.value("StoragePath");
    qDebug() << "DiskSize:" << configValues.value("DiskSize");
    qDebug() << "BlockSize:" << configValues.value("BlockSize");
}

// This will be the main disk-node class...
class DiskNode {
    Q_OBJECT

public:
    DiskNode(QWidget *parent = nullptr, const QString &host = "localhost", quint16 port = 50000);

    void sendData(const QByteArray &data);
    bool isConnected() const;

signals:
    void connectionStatusChanged(bool connected);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *socket;
};