#include "httpFormat.hpp"
#include <QDebug>

httpFormat::httpFormat(int type)
{
    Type = type;
}

QByteArray httpFormat::createFormat(int indicator, const QString& fileName, const QByteArray& fileData)
{
    QByteArray message;
    message.append("NumIndicator: " + QByteArray::number(indicator) + "\n");
    message.append("FileName: " + fileName.toUtf8() + "\n");
    message.append("Content-Length: " + QByteArray::number(fileData.size()) + "\n\n");
    message.append(fileData);
    return message;
}

void httpFormat::readMessage(const QByteArray& message)
{
    QList<QByteArray> lines = message.split('\n');
    int numIndicator = -1;
    QString fileName;
    int contentLength = 0;
    QByteArray content;

    int i = 0; // Message position

    while (i < lines.size() && !lines[i].isEmpty())
    {
        QByteArray line = lines[i];
        if (line.startsWith("NumIndicator:"))
        {
            numIndicator = line.split(':')[1].trimmed().toInt();
        }
        else if (line.startsWith("FileName:"))
        {
            fileName = line.split(':')[1].trimmed();
        }
        else if (line.startsWith("Content-Length:"))
        {
            contentLength = line.split(':')[1].trimmed().toInt();
        }
        i++;
    }

    // Empty space is deleted
    i++;

    // Show data
    qDebug() << "NumIndicator:" << numIndicator;
    qDebug() << "FileName:" << fileName;
    qDebug() << "Content-Length:" << contentLength;
    qDebug() << "Content:" << content.left(100); //first 100 bytes


    // Logic  by type
    switch (Type)
    {
        case 1: // Server->Controller
            qDebug() << "[Type 1] Server->Controller ";
            break;

        case 2: // Controller->Server
            qDebug() << "[Type 2] Controller->Server";

            break;

        case 3: // Controller->Node
            qDebug() << "[Type 3] Controller->Node";

            break;

        case 4: // Node->Controller
            qDebug() << "[Type 4] Node->Controller";
            break;

        default:
            qDebug() << "Type Error!";
            break;
    }
}

int httpFormat::getType() const
{
    return Type;
}

