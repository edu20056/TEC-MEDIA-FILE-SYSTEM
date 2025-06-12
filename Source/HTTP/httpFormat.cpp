#include "httpFormat.hpp"
#include <QDebug>

httpFormat::httpFormat()
{
    return;
}

// =========================== ActionMessage <-----> String =====================================
QString httpFormat::actionMessageToString(ActionMessage action) {
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

ActionMessage httpFormat::stringToActionMessage(const QString& str) {
    if (str == "Erase")        return ActionMessage::Erase;
    if (str == "Check")        return ActionMessage::Check;
    if (str == "Download")     return ActionMessage::Download;
    if (str == "Upload")       return ActionMessage::Upload;
    if (str == "Error")        return ActionMessage::Error;
    if (str == "MemoryStatus") return ActionMessage::MemoryStatus;
    return ActionMessage::Error; // valor por defecto seguro
}

// =========================== MessageIndicator <-----> String =====================================
QString httpFormat::messageIndicatorToString(MessageIndicator action) {
    switch (action) {
        case MessageIndicator::ServerToController:        return "ServerToController";
        case MessageIndicator::ControllerToNode:        return "ControllerToNode";
        case MessageIndicator::ControllerToServer:     return "ControllerToServer";
        case MessageIndicator::NodeToController:       return "NodeToController";
        default:                          return "Null";
    }
}

MessageIndicator httpFormat::stringToMessageIndicator(const QString& str) {
    if (str == "ServerToController")        return MessageIndicator::ServerToController;
    if (str == "ControllerToNode")        return MessageIndicator::ControllerToNode;
    if (str == "ControllerToServer")     return MessageIndicator::ControllerToServer;
    if (str == "NodeToController")       return MessageIndicator::NodeToController;
    return MessageIndicator::Null; // valor por defecto seguro
}



// =========================== CONSTRUCTOR ===========================
QByteArray httpFormat::createFormat(MessageIndicator indicator, const QString& fileName, ActionMessage& action, const QByteArray& fileData)
{
    QByteArray message;
    message.append("Indicator: " + messageIndicatorToString(indicator).toUtf8() + "\n");
    message.append("Action: " + actionMessageToString(action).toUtf8() + "\n");
    message.append("FileName: " + fileName.toUtf8() + "\n");
    message.append("Content-Length: " + QByteArray::number(fileData.size()) + "\n\n");
    message.append(fileData);
    return message;
}

void httpFormat::readMessage(const QByteArray& message) {
    // Reseat previous values
    indicator = MessageIndicator::Null;
    fileName.clear();
    contentLength = 0;
    content.clear(); 

    // Headers and body message
    int headerEnd = message.indexOf("\n\n");
    if (headerEnd == -1) {
        qDebug() << "Error: Delimitador '\\n\\n' no encontrado";
        return;
    }

    // Parse headers
    QList<QByteArray> headers = message.left(headerEnd).split('\n');
    for (const QByteArray &line : headers) {
        if (line.startsWith("Indicator:")) {
            indicator = stringToMessageIndicator(QString::fromUtf8(line.split(':')[1].trimmed()));
        } else if (line.startsWith("FileName:")) {
            fileName = line.split(':')[1].trimmed();
        } else if (line.startsWith("Content-Length:")) {
            contentLength = line.split(':')[1].trimmed().toInt();
        } else if (line.startsWith("Action:")) {
            action = stringToActionMessage(QString::fromUtf8(line.split(':')[1].trimmed()));
        }
    }

    // Get binary pdf content
    content = message.mid(headerEnd + 2); // +2 to skip \n\n

    if (contentLength != content.size()) {
        qDebug() << "Error: Content-Length no coincide con el tamaño real";
        content.clear(); 
    }
}

// ================================ GETTERS ==================================


MessageIndicator httpFormat::getIndicator() const
{
    return indicator;
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