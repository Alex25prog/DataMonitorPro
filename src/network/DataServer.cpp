#include "DataServer.h"
#include <QTcpSocket>
#include <QDebug>

DataServer::DataServer(QObject *parent)
    : QTcpServer(parent)
{
}

DataServer::~DataServer()
{
    stopServer();
}

bool DataServer::startServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Failed to start server on port" << port;
        return false;
    }
    
    m_isRunning = true;
    qDebug() << "Server started on port" << port;
    return true;
}

void DataServer::stopServer()
{
    if (m_isRunning) {
        close();
        m_isRunning = false;
        qDebug() << "Server stopped";
    }
}

void DataServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        delete socket;
        return;
    }
    
    qDebug() << "Client connected from" << socket->peerAddress().toString();
    emit clientConnected(socket->peerAddress().toString());
    
    connect(socket, &QTcpSocket::readyRead, [this, socket]() {
        QByteArray data = socket->readAll();
        emit dataReceived(QString::fromUtf8(data));
    });
    
    connect(socket, &QTcpSocket::disconnected, [this, socket]() {
        emit clientDisconnected(socket->peerAddress().toString());
        socket->deleteLater();
    });
}