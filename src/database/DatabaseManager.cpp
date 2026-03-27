
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

bool DatabaseManager::connectToPostgreSQL(const QString& host, int port,
                                           const QString& dbName,
                                           const QString& user,
                                           const QString& password)

{
    m_db = QSqlDatabase::addDatabase("QPSQL");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setUserName(user);
    m_db.setPassword(password);
    m_db.setDatabaseName(dbName);
    
    if (!m_db.open()) {
        qDebug() << "PostgreSQL connection failed:" << m_db.lastError().text();
        emit errorOccurred(m_db.lastError().text());
        return false;
    }
    qDebug() << "Connected to PostgreSQL database:" << dbName;
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);
    
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS data_points (
            id SERIAL PRIMARY KEY,
            timestamp BIGINT NOT NULL,
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

    //Для проверки, открыто ли соединение
    if (!m_db.isOpen()){
        qDebug() << "Database connection lost. Attempting to reconnect..";

        if (!m_db.open()){
            qDebug() << "Failed to reconnect:" << m_db.lastError().text();
            return false;
        }
        qDebug() << "Reconnected to PostgreSQL";
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO data_points (timestamp, type, value, unit) "
                  "VALUES ($1, $2, $3, $4)");
    
    query.bindValue(0, point.timestamp().toSecsSinceEpoch());
    query.bindValue(1, point.type());
    query.bindValue(2, point.value());
    query.bindValue(3, point.unit());
    
    if (!query.exec()) {
        qDebug() << "Failed to save data point:" << query.lastError().text();
        return false;
    }
    qDebug() << "Saved point:" << point.type() << point.value() << point.unit();

    return true;
}

QList<DataPoint> DatabaseManager::loadDataPoints(const QDateTime& from, const QDateTime& to)
{
    QList<DataPoint> points;

    if (!m_db.isOpen()){
        qDebug() << "Database not open, cannot load data";
        return points;
    }
    QSqlQuery query(m_db);
    
    query.prepare("SELECT timestamp, type, value, unit FROM data_points "
                  "WHERE timestamp BETWEEN $1 AND $2 "
                  "ORDER BY timestamp DESC");
    
    query.bindValue(0, from.toSecsSinceEpoch());
    query.bindValue(1, to.toSecsSinceEpoch());
    
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
    
    qDebug() << "Loaded" << points.size() << "points from PostgreSQL";
    return points;
}