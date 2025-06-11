#ifndef XMLREADER_HPP
#define XMLREADER_HPP

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>

enum class xmlTag {
    NodeID,
    ControllerIP,
    ControllerPort,
    StoragePath,
    DiskSize,
    BlockSize,
    TotalSpace
};

QString configRead(const QString &filePath, xmlTag tag);

#endif
