#ifndef HTTPSFORMAT_HPP
#define HTTPSFORMAT_HPP

#include <QString>
#include <QByteArray>

class httpFormat
{
private:
    int Type; // 1: Server->Controller, 2: Controller->Server,  3: Controller->Node, 4: Node->Controller

    // Information of last message that was read
    int numIndicator; 
    QString fileName;
    QString action;
    int contentLength;
    QByteArray content;
public:
    httpFormat(int type); 

    // indicator 1:Search, 2:Delete, 3:Upload
    QByteArray createFormat(int indicator, const QString& fileName, QString& action,const QByteArray& fileData);
    void readMessage(const QByteArray& message);

    void SetType(int newType);

    int getType() const;
    int getIndicator() const;
    QString getFileName() const;
    int getContentLenght() const;
    QByteArray getContent() const;
    QString getAction() const;
};

#endif
