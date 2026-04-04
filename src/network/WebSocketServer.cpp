#include "WebSocketServer.h"
#include <QDebug>

WebSocketServer::WebSocketServer(QObject* parent)
    : QWebSocketServer("DataMonitorPro", QWebSocketServer::NonSecureMode, parent)
{
    connect(this, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
}

WebSocketServer::~WebSocketServer()
{
    stopServer();
}

bool WebSocketServer::startServer(quint16 port)
{
    if (listen(QHostAddress::Any, port)) {
        m_isRunning = true;
        qDebug() << "WebSocket server started on port" << port;
        return true;
    }
    qDebug() << "Failed to start WebSocket server on port" << port;
    return false;
}

void WebSocketServer::stopServer()
{
    if (m_isRunning) {
        for (QWebSocket* client : m_clients) {
            client->close();
            client->deleteLater();
        }
        m_clients.clear();
        close();
        m_isRunning = false;
        qDebug() << "WebSocket server stopped";
    }
}

void WebSocketServer::onNewConnection()
{
    QWebSocket* socket = nextPendingConnection();
    if (!socket) return;

    m_clients.append(socket);
    qDebug() << "Client connected from" << socket->peerAddress().toString();
    emit clientConnected();

    connect(socket, &QWebSocket::textMessageReceived,
        this, &WebSocketServer::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected,
        this, &WebSocketServer::onSocketDisconnected);
}

void WebSocketServer::onTextMessageReceived(const QString& message)
{
    qDebug() << "Received:" << message;
    emit dataReceived(message);
}

void WebSocketServer::onSocketDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (socket) {
        m_clients.removeAll(socket);
        socket->deleteLater();
        qDebug() << "Client disconnected";
        emit clientDisconnected();
    }
}