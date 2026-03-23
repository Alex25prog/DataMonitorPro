#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include "../core/DataPoint.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool initialize(const QString& connectionName = "datamonitor");
    bool saveDataPoint(const DataPoint& point);
    QList<DataPoint> loadDataPoints(const QDateTime& from, const QDateTime& to);
    
signals:
    void errorOccurred(const QString& error);

private:
    QSqlDatabase m_db;
    bool createTables();
};

#endif // DATABASEMANAGER_H