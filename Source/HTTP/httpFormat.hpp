#ifndef HTTPSFORMAT_HPP
#define HTTPSFORMAT_HPP

#include <QString>
#include <QByteArray>

enum class ActionMessage {
    Erase,
    Check,
    Download,
    Upload,
    Error,
    MemoryStatus,
    Null

};

enum class MessageIndicator {
    ServerToController,
    ControllerToServer,
    ControllerToNode,
    NodeToController,
    Null
};

class httpFormat
{
private:
    // Information of last message that was read
    MessageIndicator indicator; 
    QString fileName;
    ActionMessage action;
    int contentLength;
    QByteArray content;

public:
    httpFormat(); 

    // indicator 1:Search, 2:Delete, 3:Upload
    QByteArray createFormat(MessageIndicator indicator, const QString& fileName, ActionMessage& action,const QByteArray& fileData);
    void readMessage(const QByteArray& message);

    void SetType(int newType);

    int getType() const; // <------
    MessageIndicator getIndicator() const; // <------
    QString getFileName() const;
    int getContentLenght() const;
    QByteArray getContent() const;
    ActionMessage getAction() const;

    // ActionMessage <-----> String
    QString actionMessageToString(ActionMessage action);
    ActionMessage stringToActionMessage(const QString& str);

    // MessageIndicator <-----> String
    QString messageIndicatorToString(MessageIndicator action);
    MessageIndicator stringToMessageIndicator(const QString& str);
};

#endif
