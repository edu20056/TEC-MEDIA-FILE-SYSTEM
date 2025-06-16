#include "interface.hpp"

App::App(QWidget *parent, const QString &host, quint16 port) : QWidget(parent), socket(new QTcpSocket(this)) {

    connect(socket, &QTcpSocket::readyRead, this, &App::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &App::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &App::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &App::onError);

    socket->connectToHost(host, port);

    // Layouts
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    QVBoxLayout *tableLayout = new QVBoxLayout();

    // Buttons
    btnErase = new QPushButton("Borrar PDF");
    btnUpload = new QPushButton("Subir PDF");
    btnDownload = new QPushButton("Descargar PDF");
    btnCheck = new QPushButton("Revisar existencia");

    // Line messages
    showInformation = new QLabel("Esperando acción del usuario...");
    lineEditPDFName = new QLineEdit();
    showInformation->move(100, 200);
    lineEditPDFName->setPlaceholderText("Nombre PDF");

    // Añadir al layout de botones
    buttonLayout->addWidget(btnUpload);
    buttonLayout->addWidget(btnDownload);
    buttonLayout->addWidget(btnCheck);
    buttonLayout->addWidget(btnErase);
    buttonLayout->addWidget(lineEditPDFName);
    buttonLayout->addWidget(showInformation);
    buttonLayout->addStretch(); // Espacio flexible abajo

    // Crear tabla y añadir al layout de tabla
    setupNodeStatusTable(); // crea `nodeStatusTable`
    tableLayout->addWidget(nodeStatusTable);
    QHeaderView *header = nodeStatusTable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    // Añadir ambos al layout principal
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(tableLayout);

    // Aplicar layout a la ventana
    setLayout(mainLayout);

    // Estilo
    this->setStyleSheet("background-color: lightcoral;");
    QSize screenSize = qApp->primaryScreen()->availableGeometry().size();
    resize(screenSize.width() * 0.7, screenSize.height() * 0.6);

    connect(btnErase, &QPushButton::clicked, this, &App::erasePDF); // Erase
    connect(btnUpload, &QPushButton::clicked, this, &App::UploadPDF); // Upload pdf
    connect(btnDownload, &QPushButton::clicked, this, &App::Download); // Download
    connect(btnCheck, &QPushButton::clicked, this, &App::CheckExistent); // Download
}

void App::changeInformationMessage(QLabel* textBox, const QString& nuevoTexto) {
    textBox->setText(nuevoTexto);
}
// ======================== CONNECTION FUNCTIONS ============================================

void App::onConnected() {

    qDebug() << "Conectado al servidor";
    emit connectionStatusChanged(true);
}

void App::onDisconnected() {

    qDebug() << "Desconectado del servidor";
    emit connectionStatusChanged(false);
}

void App::onError(QAbstractSocket::SocketError error) {

    Q_UNUSED(error);
    qDebug() << "Error de socket:" << socket->errorString();
}


bool App::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

void App::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    // Acumulate data on buffer.
    buffers[client] += client->readAll();

    // Procesar mensajes completos
    while (buffers[client].size() >= 4) { // We asume that the first 4 bytes are for message size
        // Read message size (first 4 bytes)
        quint32 messageLength;
        QDataStream stream(buffers[client]);
        stream >> messageLength;

        // Verify if message is complete
        if (buffers[client].size() - static_cast<qint64>(sizeof(quint32)) >= static_cast<qint64>(messageLength)) {
            // Extract message
            QByteArray completeMessage = buffers[client].mid(4, messageLength);
            buffers[client].remove(0, 4 + messageLength);

            // Message procesor
            messageFormat.readMessage(completeMessage);
            if (messageFormat.getIndicator() == MessageIndicator::ControllerToServer)
            {
                if (messageFormat.getAction() == ActionMessage::Upload)
                {
                    qDebug() << "Se intento subir el pdf: " + messageFormat.getFileName() + ". Se obtuvo como resultado: " + messageFormat.getContent();
                    QString nombre = messageFormat.getFileName().split("_").at(0); 
                    changeInformationMessage(showInformation, "Se subió el pdf: " + nombre);
                }

                else if (messageFormat.getAction() == ActionMessage::Erase)
                {
                    qDebug() << "Se intento borrar el pdf: " + messageFormat.getFileName() + ". Se obtuvo como resultado: " + messageFormat.getContent();
                    changeInformationMessage(showInformation, messageFormat.getContent());
                }

                else if (messageFormat.getAction() == ActionMessage::Download)
                {
                    qDebug() << "Se descarga el file: " + messageFormat.getFileName();
                    changeInformationMessage(showInformation, messageFormat.getContent());
                }

                else if (messageFormat.getAction() == ActionMessage::Check)
                {
                    qDebug() << "Se checkeo el estado de: " + messageFormat.getFileName() + ". Estado: " + messageFormat.getContent();
                    changeInformationMessage(showInformation, messageFormat.getContent() + " Nombre del pdf: " + messageFormat.getFileName());
                }
                else if (messageFormat.getAction() == ActionMessage::Error)
                {
                    changeInformationMessage(showInformation, messageFormat.getContent());
                
                }
                

                else if (messageFormat.getAction() == ActionMessage::MemoryStatus) {
                    QByteArray raw = messageFormat.getContent();
                    QString status = QString::fromUtf8(raw);  // Asegúrate de esto

                    qDebug().noquote() << ">> Status recibido:\n" << status;

                    int nodeID = extractNodeID(status);
                    qDebug() << ">> ID extraído:" << nodeID;

                    QStringList fileList;
                    if (status.toLower().contains("connected: no")) {
                        qDebug() << "[CONTAINS NO]" ;
                        fileList << "DISCONNECTED";
                    } else {
                        fileList = extractFileNames(status);
                    }

                    qDebug() << ">> Archivos extraídos o estado:" << fileList;

                    updateNodeStatus(nodeID, fileList);
                }
            }

            else
            {
                qDebug() << "Se recibio un mensaje no deseado para GUI";
            }
        }
    }
}

// =========================== SEND INFORMATION WITH BUTTONS =================================

void App::sendData(const QByteArray &data) {
    if (!isConnected()) {
        qDebug() << "No conectado, no se puede enviar datos";
        return;
    }

    // Prepend message length (4 bytes) before sending
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << quint32(data.size()); // Write message size as a 4-byte header
    block.append(data); // Append the actual data

    qint64 bytesWritten = socket->write(block);
    if (bytesWritten == -1) {
        qDebug() << "Error al enviar datos:" << socket->errorString();
        return;
    }

    if (!socket->waitForBytesWritten(1000)) {
        qDebug() << "Timeout al enviar datos";
    }
}
void App::erasePDF()
{
    QString pdfName = lineEditPDFName->text();
    if (pdfName.isEmpty()) {
        qDebug() << "Debes ingresar un nombre de PDF.";
        changeInformationMessage(showInformation, "Al eliminar un pdf, se debe colocara el nombre del archivo que desea eliminar.");
    
        return;
    }
    
    ActionMessage action = ActionMessage::Erase;   
    QByteArray _ = "";
    QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController,pdfName, action, _);
    sendData(message);
}

void App::UploadPDF() {
    QString fileName = QFileDialog::getOpenFileName(this, "Seleccionar archivo PDF", QDir::homePath(), "Archivos PDF (*.pdf)");
    if (fileName.isEmpty())
    {
        changeInformationMessage(showInformation, "El espacio de texto esta vacio.");
        return;
    }


    QFile file(fileName);
    QFileInfo fileIndo(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No se pudo abrir el archivo:" << fileName;
        changeInformationMessage(showInformation, "El archivo seleccionado " + fileName +" no se puede abrir vacio.");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        qDebug() << "El archivo PDF está vacío:"  << fileName;
        changeInformationMessage(showInformation, "El archivo seleccionado " + fileName +" esta vacio.");
        
        return;
    }

    QString baseName = fileIndo.completeBaseName();
    ActionMessage action = ActionMessage::Upload; 
    QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController, baseName, action, data);
    sendData(message);
}


void App::Download() {

    QString fileName = lineEditPDFName->text(); 

    if (!fileName.isEmpty()) {
        qDebug() << "Archivo PDF seleccionado:" << fileName;

        ActionMessage action = ActionMessage::Download; 
        QByteArray _ = "";
        QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController,fileName, action, _);
        sendData(message);
    } else {
        qDebug() << "No se seleccionó ningún archivo." << fileName;
        changeInformationMessage(showInformation, "Al descargar, se debe colocara el nombre del archivo que desea descargar.");
    }
}

void App::CheckExistent() {

    QString fileName = lineEditPDFName->text(); 

    if (!fileName.isEmpty()) {
        qDebug() << "Archivo PDF seleccionado:" << fileName;

        ActionMessage action = ActionMessage::Check; 
        QByteArray _ = "";
        QByteArray message = messageFormat.createFormat(MessageIndicator::ServerToController,fileName, action, _);
        sendData(message);
    } else {
        qDebug() << "No se seleccionó ningún archivo." << fileName;
        changeInformationMessage(showInformation, "Al revisar pdf, se debe colocara el nombre del archivo que desea revisar.");
    
    }
}

void App::setupNodeStatusTable() {
    nodeStatusTable = new QTableWidget(this);
    nodeStatusTable->setRowCount(0);
    nodeStatusTable->setColumnCount(4);
    QStringList headers;
    headers << "N1" << "N2" << "N3" << "N4";
    nodeStatusTable->setHorizontalHeaderLabels(headers);
    nodeStatusTable->horizontalHeader()->setStretchLastSection(true);
    nodeStatusTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    nodeStatusTable->setSelectionMode(QAbstractItemView::NoSelection);
    nodeStatusTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    nodeStatusTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    nodeStatusTable->setWordWrap(true);
}

void App::updateNodeStatus(int nodeID, const QStringList &fileList) {
    if (nodeID < 1 || nodeID > 4) return;
    int column = nodeID - 1;

    if (fileList.size() == 1 && fileList[0] == "DISCONNECTED") {

        for (int i = 0; i < nodeStatusTable->rowCount(); ++i)
            nodeStatusTable->setItem(i, column, nullptr);

        nodeStatusTable->setColumnHidden(column, true);
        return;
    }

    nodeStatusTable->setColumnHidden(column, false);

    for (int i = 0; i < nodeStatusTable->rowCount(); ++i)
        nodeStatusTable->setItem(i, column, nullptr);

    int neededRows = fileList.size();
    if (nodeStatusTable->rowCount() < neededRows)
        nodeStatusTable->setRowCount(neededRows);

    for (int i = 0; i < fileList.size(); ++i) {
        QTableWidgetItem *item = new QTableWidgetItem(fileList[i]);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setToolTip(fileList[i]);
        nodeStatusTable->setItem(i, column, item);
    }
}

int App::extractNodeID(const QString &status) {
    QRegularExpression idRegex("Node ID: (\\d+)");
    QRegularExpressionMatch match = idRegex.match(status);
    return match.hasMatch() ? match.captured(1).toInt() : -1;
}

QStringList App::extractFileNames(const QString &status) {
    QStringList lines = status.split('\n', Qt::SkipEmptyParts);
    QStringList files;
    for (const QString &line : lines) {
        if (line.startsWith(" - ")) files << line.mid(3);
    }
    return files;
}
