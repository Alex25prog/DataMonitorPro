#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include "../core/DataPoint.h"
#include <QList>
#include <QDateTime>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool connectToPostgreSQL(const QString& host = "localhost",
                             int potr = 5432,
                             const QString& dbName = "datamonitor",
                             const QString& user = "postgres",
                             const QString& password = ""  );



    bool saveDataPoint(const DataPoint& point);
    QList<DataPoint> loadDataPoints(const QDateTime& from, const QDateTime& to);
    
signals:
    void errorOccurred(const QString& error);

private:
    QSqlDatabase m_db;
    bool createTables();
};

#endif // DATABASEMANAGER_H