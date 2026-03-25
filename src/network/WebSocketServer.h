#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QObject>
#include <QList>

class WebSocketServer : public QWebSocketServer
{
    Q_OBJECT

public:
    explicit WebSocketServer(QObject* parent = nullptr);
    ~WebSocketServer();

    bool startServer(quint16 port);
    void stopServer();

signals:
    void dataReceived(const QString& data);
    void clientConnected();
    void clientDisconnected();

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString& message);
    void onSocketDisconnected();

private:
    QList<QWebSocket*> m_clients;
    bool m_isRunning = false;
};

#endif // WEBSOCKETSERVER_H