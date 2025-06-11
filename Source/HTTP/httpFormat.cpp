#include "httpFormat.hpp"
#include <QDebug>

httpFormat::httpFormat(int type)
{
    Type = type;
}

// =========================== ActionMessage <-----> String =====================================
QString actionMessageToString(ActionMessage action) {
    switch (action) {
        case ActionMessage::Erase:        return "Erase";
        case ActionMessage::Check:        return "Check";
        case ActionMessage::Download:     return "Download";
        case ActionMessage::Upload:       return "Upload";
        case ActionMessage::Error:        return "Error";
        case ActionMessage::MemoryStatus: return "MemoryStatus";
        default:                          return "Unknown";
    }
}

ActionMessage stringToActionMessage(const QString& str) {
    if (str == "Erase")        return ActionMessage::Erase;
    if (str == "Check")        return ActionMessage::Check;
    if (str == "Download")     return ActionMessage::Download;
    if (str == "Upload")       return ActionMessage::Upload;
    if (str == "Error")        return ActionMessage::Error;
    if (str == "MemoryStatus") return ActionMessage::MemoryStatus;
    return ActionMessage::Error; // valor por defecto seguro
}


// =========================== CONSTRUCTOR ==========================================================================
QByteArray httpFormat::createFormat(int indicator, const QString& fileName, ActionMessage& action, const QByteArray& fileData)
{
    QByteArray message;
    message.append("NumIndicator: " + QByteArray::number(indicator) + "\n");
    message.append("Action: " + actionMessageToString(action).toUtf8() + "\n");
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
            action = stringToActionMessage(QString::fromUtf8(line.split(':')[1].trimmed()));
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

ActionMessage httpFormat::getAction() const 
{
    return action;
}