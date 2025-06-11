#include "diskNode.hpp"

int main(int argc, char *argv[]){

    QCoreApplication core(argc, argv);

    // ===================== INPUT MANAGEMENT ========================
    if (argc < 2) {
        qDebug() << "Usage: TECMFS-NODE <config_file.xml>";
        return 1;
    }
    QString xmlPath = argv[1];

    QString portStr = configRead(xmlPath, xmlTag::ControllerPort);
    quint16 port = portStr.toUShort();
    QString ip = configRead(xmlPath, xmlTag::ControllerIP);
    QString path = configRead(xmlPath, xmlTag::StoragePath);
    QString idStr = configRead(xmlPath, xmlTag::NodeID);
    quint16 id = idStr.toUShort();

    // ===================== NODE INITIALIZATION =====================
    if(id == 0) return -1; // Prevents invalid nodes initialization
    
    DiskNode node(nullptr, ip, port, path, id);
    return core.exec();
}
