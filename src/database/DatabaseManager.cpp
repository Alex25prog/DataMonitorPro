#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize(const QString& connectionName)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    m_db.setDatabaseName("datamonitor.db");
    
    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        emit errorOccurred(m_db.lastError().text());
        return false;
    }
    
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);
    
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS data_points (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            type TEXT NOT NULL,
            value REAL NOT NULL,
            unit TEXT
        )
    )";
    
    if (!query.exec(createTable)) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }
    
    // Создаем индекс для быстрого поиска по времени
    query.exec("CREATE INDEX IF NOT EXISTS idx_timestamp ON data_points(timestamp)");
    
    return true;
}

bool DatabaseManager::saveDataPoint(const DataPoint& point)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO data_points (timestamp, type, value, unit) "
                  "VALUES (:timestamp, :type, :value, :unit)");
    
    query.bindValue(":timestamp", point.timestamp().toSecsSinceEpoch());
    query.bindValue(":type", point.type());
    query.bindValue(":value", point.value());
    query.bindValue(":unit", point.unit());
    
    if (!query.exec()) {
        qDebug() << "Failed to save data point:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<DataPoint> DatabaseManager::loadDataPoints(const QDateTime& from, const QDateTime& to)
{
    QList<DataPoint> points;
    QSqlQuery query(m_db);
    
    query.prepare("SELECT timestamp, type, value, unit FROM data_points "
                  "WHERE timestamp BETWEEN :from AND :to "
                  "ORDER BY timestamp DESC");
    
    query.bindValue(":from", from.toSecsSinceEpoch());
    query.bindValue(":to", to.toSecsSinceEpoch());
    
    if (!query.exec()) {
        qDebug() << "Failed to load data points:" << query.lastError().text();
        return points;
    }
    
    while (query.next()) {
        QDateTime timestamp;
        timestamp.setSecsSinceEpoch(query.value(0).toLongLong());
        
        DataPoint point(
            timestamp,
            query.value(1).toString(),
            query.value(2).toDouble(),
            query.value(3).toString()
        );
        
        points.append(point);
    }
    
    return points;
}