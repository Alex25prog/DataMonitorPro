#include "MainController.h"
#include "../core/SecretManager.h"
#include <QQmlContext>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>

     MainController::MainController(QQmlApplicationEngine* engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_dataModel(new DataModel(this))
    , m_server(new WebSocketServer(this))
    , m_database(new DatabaseManager(this))
    , m_processor(new DataProcessor(this))
    , m_exporter (new ReportExporter(this))
    , m_weatherFetcher(new WeatherFetcher(this))

{
    // Подключаем сигналы
    connect(m_processor, &DataProcessor::dataProcessed, this, &MainController::updateChart);
    connect(m_server, &WebSocketServer::dataReceived, this, &MainController::onDataReceived);
    connect(m_processor, &DataProcessor::dataProcessed, this, &MainController::onDataProcessed);
    connect(m_processor, &DataProcessor::dataProcessed, m_dataModel, &DataModel::addDataPoint);
    connect(m_exporter, &ReportExporter::exportFinished,
            [](bool success, const QString& filePath, const QString& error){
       if (success) {
           qDebug() << "Export successful:" << filePath;
       } else {
           qDebug() << "Export failed:" << error;
       }
    });



    // Инициализируем базу данных(Берем пароль БД из переменного окружения)
    //QString dbPassword = qgetenv("DB_PASSWORD");
    //if (dbPassword.isEmpty()) {
        //qDebug() << "WARNING: DB_PASSWORD enviroment variable not set! Using default (not recommended)";
        //dbPassword = ""; // Можно оставить пустым
    //}

    //Загружаем API ключ через SecretManager
    QString apiKey = SecretManager::getWeatherApiKey();
    if (!apiKey.isEmpty()) {
        m_weatherFetcher->setApiKey(apiKey);
        qDebug() << "Weather API key loaded from secure storage";
    } else {
        qDebug() << "=================================";
        qDebug() << "WARNING: Weather API key not found!";
        qDebug() << "Please create file: .secrets/weather_api.key";
        qDebug() << "With your API key from openWeatherMap";
        qDebug() << "=================================";
    }

    //Подключаем сигналы погоды
    connect(m_weatherFetcher, &WeatherFetcher::weatherDataReceived,
            this, &MainController::onWeatherDataReceived);
    connect(m_weatherFetcher, &WeatherFetcher::errorOccurred,
            [](const QString& error) {qDebug() << "Weather error:" << error;});

        // Загружаем пароль БД из безопасной папки
    QString dbPassword = SecretManager::getDbPassword();
    if (dbPassword.isEmpty()) {
        qDebug() << "WARNING: Database password not found in secure storage!";
        dbPassword = ""; //Используем пустой пароль
    }

    // Остальные параметры БД (можно тоже вынести в сектреты при желании)
    const QString DB_HOST = "localhost";
    const int DB_PORT = 5432;
    const QString DB_NAME = "datamonitor";
    const QString DB_USER = "postgres";


    if (!m_database->connectToPostgreSQL(DB_HOST, DB_PORT, DB_NAME, DB_USER, dbPassword)) {
        qDebug() << "Failed to connect to PostgreSQL";
        qDebug() << "Please ensure PostgreSQL is running and database 'datamonitor' exists";
    }else {
        qDebug() << "PostgreSQL connected successfully";
    }


    // Регистрируем модель для QML
    qmlRegisterUncreatableType<DataModel>("com.datamonitor", 1, 0, "DataModel", "Cannot create DataModel in QML");

    // Делаем контроллер доступным из QML
    m_engine->rootContext()->setContextProperty("controller", this);
}

//Деструктор
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
    //static double startTime = 0;
    static int pointIndex = 0; // добавляем счетчик точек

    //double currentTime = point.timestamp().toMSecsSinceEpoch() / 1000.0; // в секундах


    //if (startTime == 0) {
        //startTime = currentTime;
    //}

    double value = point.value();

    //Нормализация значений для отображения на одном графике
    if (point.type() == "temperature") {
        value = point.value() * 10; //10C = 100
            }else if (point.type() == "pressure") {
        value = point.value() / 10.26; // 1026 hPa = 100
            } else if (point.type() == "humidity") {
                value = point.value(); // 50% = 50
    }
        qDebug() << "updateChart called:" << point.type() << "time=" << pointIndex << "value=" << value;
        //Оставляем сигнал для QML
        emit chartDataReceived(pointIndex, value, point.type());
        pointIndex++; // 0, 1, 2, 3...
}

void MainController::onWeatherDataReceived(const QString& type, double value, const QString& unit)
{
    DataPoint point(QDateTime::currentDateTime(), type, value, unit);
    m_processor->processDataPoint(point);
    m_database->saveDataPoint(point);
    updateChart(point); //Для обновления графика
    qDebug() << "Weather saved:" << type << value << unit;
}
void MainController::exportToCSV()//Метод экспорта в CSV
{
    //Формируем имя файла с текущей датой и временем
    QString fileName = QString("DataMonitor_Export_%1.csv")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    //Путь к папке Документы
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                       + "/" + fileName;


    //Вызываем экспорт через ReportExporter
    m_exporter->exportCurrentData(m_dataModel, filePath, "csv");

}

void MainController::exportToPDF()
{

    //Формируем имя файла с текущей датой и временем
    QString fileName = QString("DataMonitor_Report_%1.pdf")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));


    //Путь к папке документы
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                       + "/" + fileName;

    //Вызываем экспорт через ReportExporter
    m_exporter->exportCurrentData(m_dataModel, filePath, "pdf");
}

void MainController::startWeather()//Метод старта
{
    if (m_weatherFetcher){
        m_weatherFetcher->startFetching(300);//обновления каждые 300сек(5мин)
        m_weatherRunning = true;
        emit weatherRunningChanged();//Отсылаем сигнал в QML для изменения кнопки
        qDebug() << "Weather monitoring started (interval: 300 sec)";
    }
}

void MainController::stopWeather()//Метод стоп
{
    if (m_weatherFetcher){
        m_weatherFetcher->stopFetching();
        m_weatherRunning = false;
        emit weatherRunningChanged();//Отсылаем сигнал в QML для изменения кнопки
        qDebug() << "Weather monitoring stopped";
    }
}