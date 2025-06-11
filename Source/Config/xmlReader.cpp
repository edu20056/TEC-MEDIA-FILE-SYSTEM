#include "xmlReader.hpp"

QString configRead(const QString &filePath, xmlTag tag) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "ERROR: XML couldn't be opened..." << filePath;
        return "";
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

    switch (tag) {
        case xmlTag::NodeID:          return configValues.value("NodeID");
        case xmlTag::ControllerIP:    return configValues.value("ControllerIP");
        case xmlTag::ControllerPort:  return configValues.value("ControllerPort");
        case xmlTag::StoragePath:     return configValues.value("StoragePath");
        case xmlTag::DiskSize:        return configValues.value("DiskSize");
        case xmlTag::BlockSize:       return configValues.value("BlockSize");
        case xmlTag::TotalSpace:      return configValues.value("TotalSpace");
        default:                      return {};
    }
}