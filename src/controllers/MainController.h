#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "../models/DataModel.h"
#include "../network/WebSocketServer.h"
#include "../database/DatabaseManager.h"
#include "../core/DataProcessor.h"

class MainController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DataModel* dataModel READ dataModel CONSTANT)
    Q_PROPERTY(bool isServerRunning READ isServerRunning NOTIFY serverRunningChanged)

public:
    explicit MainController(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    ~MainController();
    
    DataModel* dataModel() const { return m_dataModel; }
    bool isServerRunning() const { return m_serverRunning; }
    
    Q_INVOKABLE bool startServer(quint16 port = 8080);
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void loadHistory(const QDateTime& from, const QDateTime& to);
    
signals:
    void serverRunningChanged();
    void chartDataReceived(qreal timestamp, qreal value);

private slots:
    void onDataReceived(const QString& data);
    void onDataProcessed(const DataPoint& point);
    void updateChart(const DataPoint& point);

private:
    QQmlApplicationEngine* m_engine;
    DataModel* m_dataModel;
    WebSocketServer* m_server;
    DatabaseManager* m_database;
    DataProcessor* m_processor;
    bool m_serverRunning = false;
    
    DataPoint parseData(const QString& data);
};

#endif // MAINCONTROLLER_H