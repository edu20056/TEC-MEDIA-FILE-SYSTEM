#include "httpFormat.hpp"
#include <QDebug>

httpFormat::httpFormat(int type)
{
    Type = type;
}

QByteArray httpFormat::createFormat(int indicator, const QString& fileName, QString& action, const QByteArray& fileData)
{
    QByteArray message;
    message.append("NumIndicator: " + QByteArray::number(indicator) + "\n");
    message.append("Action: " + action.toUtf8() + "\n");
    message.append("FileName: " + fileName.toUtf8() + "\n");
    message.append("Content-Length: " + QByteArray::number(fileData.size()) + "\n\n");
    message.append(fileData);
    return message;
}

void httpFormat::readMessage(const QByteArray& message)
{
    QList<QByteArray> lines = message.split('\n');

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
        else if (line.startsWith("Action:"))
        {
            action = line.split(':')[1].trimmed();
        };
        i++;
    }

    // Show data
    qDebug() << "NumIndicator:" << numIndicator;
    qDebug() << "FileName:" << fileName;
    qDebug() << "Content-Length:" << contentLength;
    qDebug() << "Content:" << content.left(100); //first 100 bytes
}

// ================================== SETTERS ===================================

void httpFormat::SetType(int newType)
{
    Type = newType;
}

// ================================ GETTERS ==================================

int httpFormat::getType() const
{
    return Type;
}

int httpFormat::getIndicator() const
{
    return numIndicator;
}

QString httpFormat::getFileName() const
{
    return fileName;
}

int httpFormat::getContentLenght() const
{
    return contentLength;
}

QByteArray httpFormat::getContent() const
{
    return content;
}

QString httpFormat::getAction() const 
{
    return action;
}