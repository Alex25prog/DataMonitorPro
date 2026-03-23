#ifndef DATASERVER_H
#define DATASERVER_H

#include <QTcpServer>
#include <QObject>

class DataServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit DataServer(QObject *parent = nullptr);
    ~DataServer();
    
    bool startServer(quint16 port);
    void stopServer();

signals:
    void dataReceived(const QString& data);
    void clientConnected(const QString& address);
    void clientDisconnected(const QString& address);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    bool m_isRunning = false;
};

#endif // DATASERVER_H