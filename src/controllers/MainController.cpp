#include "MainController.h"
#include <QQmlContext>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

MainController::MainController(QQmlApplicationEngine* engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_dataModel(new DataModel(this))
    , m_server(new WebSocketServer(this))
    , m_database(new DatabaseManager(this))
    , m_processor(new DataProcessor(this))
{
    // Подключаем сигналы
    connect(m_server, &WebSocketServer::dataReceived, this, &MainController::onDataReceived);
    connect(m_processor, &DataProcessor::dataProcessed, this, &MainController::onDataProcessed);
    connect(m_processor, &DataProcessor::dataProcessed, m_dataModel, &DataModel::addDataPoint);


    // Инициализируем базу данных
    if (!m_database->connectToPostgreSQL("localhost", 5432, "datamonitor", "postgres", "12345")) {
        qDebug() << "Failed to connect to PostgreSQL";
    }else {
        qDebug() << "PostgreSQL ready";
    }
    //WeatherFetcher
    m_weatherFetcher = new WeatherFetcher(this);
    m_weatherFetcher->setApiKey("15dcc84205e66754254850365e35c659"); //API key

    connect(m_weatherFetcher, &WeatherFetcher::weatherDataReceived,
            this, &MainController::onWeatherDataReceived);

    connect(m_weatherFetcher, &WeatherFetcher::errorOccurred,
            [](const QString& error) { qDebug() << "Weather error:" << error;});
  //запускаем получение погоды каждые 10 минут (600 секунд)

    m_weatherFetcher->startFetching(600);

    // Регистрируем модель для QML
    qmlRegisterUncreatableType<DataModel>("com.datamonitor", 1, 0, "DataModel", "Cannot create DataModel in QML");

    // Делаем контроллер доступным из QML
    m_engine->rootContext()->setContextProperty("controller", this);
}

MainController::~MainController()
{
    stopServer();
}

bool MainController::startServer(quint16 port)
{
    if (m_server->startServer(port)) {
        m_serverRunning = true;
        emit serverRunningChanged();
        return true;
    }
    return false;
}

void MainController::stopServer()
{
    m_server->stopServer();
    m_serverRunning = false;
    emit serverRunningChanged();
}

void MainController::onDataReceived(const QString& data)
{
    qDebug() << "Received data:" << data;
    DataPoint point = parseData(data);
    if (point.isValid()) {
        m_processor->processDataPoint(point);
        if (!m_database->saveDataPoint(point)){
            qDebug() << "Failed to save point to database";
        }
    }
}

void MainController::onDataProcessed(const DataPoint& point)
{
    qDebug() << "Processed point:" << point.toString();
}

void MainController::loadHistory(const QDateTime& from, const QDateTime& to)
{
    QList<DataPoint> history = m_database->loadDataPoints(from, to);
    m_dataModel->clear();
    for (const DataPoint& point : history) {
        m_dataModel->addDataPoint(point);
    }
}

DataPoint MainController::parseData(const QString& data)
{
    // Парсим JSON
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (doc.isNull()) {
        qDebug() << "Invalid JSON";
        return DataPoint();
    }

    QJsonObject obj = doc.object();

    QDateTime timestamp = QDateTime::currentDateTime();
    if (obj.contains("timestamp")) {
        timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
    }

    QString type = obj["type"].toString();
    double value = obj["value"].toDouble();
    QString unit = obj["unit"].toString();

    return DataPoint(timestamp, type, value, unit);
}
void MainController::updateChart(const DataPoint& point)
{
    emit chartDataReceived(point.timestamp().toMSecsSinceEpoch(), point.value());
}

void MainController::onWeatherDataReceived(const QString& type, double value, const QString& unit)
{
    DataPoint point(QDateTime::currentDateTime(), type, value, unit);
    m_processor->processDataPoint(point);
    m_database->saveDataPoint(point);
    qDebug() << "Weather saved:" << type << value << unit;
}