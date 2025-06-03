#include <QCoreApplication>
#include "controller.hpp"

int main(int argc, char *argv[]){

    QCoreApplication app(argc, argv);
   
    quint16 port = 50000;
    NodeController server(nullptr, port);

    return app.exec();
}
