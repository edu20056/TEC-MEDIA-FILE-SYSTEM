#include "diskNode.hpp"

// ======================== XML READER ============================================
#include <QXmlStreamReader>

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
// =================================================================================

int main(int argc, char *argv[]){

    QCoreApplication core(argc, argv);
   
    quint16 port = 50000;
    DiskNode node(nullptr, "localhost", port);

    return core.exec();
}
