#include "controller.hpp"

int main(int argc, char *argv[]){

    QCoreApplication app(argc, argv);

    // ===================== INPUT MANAGEMENT ========================
    if (argc < 2) {
        qDebug() << "Usage: TECMFS-NODE <config_file.xml>";
        return 1;
    }
    QString xmlPath = argv[1];

    QString portStr = configRead(xmlPath, xmlTag::ControllerPort);
    quint16 port = portStr.toUShort();
    QString blockSize = configRead(xmlPath,xmlTag::TotalSpace);
    quint64 size = 1000000 * blockSize.toUShort();  

    NodeController server(nullptr, port, size);

    return app.exec();
}
