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
    //m_engine->rootContext()->setContextProperty("controller", this);
}

//Деструктор
MainController::~MainController()
{
    qDebug() << "===MainController destructor SRART ===";

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
    qDebug() << "Loading history from" << from << "to" << to;

    if (!m_database) {
        qDebug() << "Databse is null!";
        return;
    }

    QList<DataPoint> history = m_database->loadDataPoints(from, to);
    qDebug() << "Loaded" << history.size() << "points from database";

    if (history.isEmpty()) {
        qDebug() << "History is empty";
        return;
    }


    // Очищаем текущие данные
    if (m_dataModel) {
        m_dataModel->clear();
    }
    // Сбрасываем счетчик точек
     m_pointIndex = 0;

    // Очищаем график
    emit clearGraphRequested();


    for (const DataPoint& point : history) {
        if (m_dataModel) {
            m_dataModel->addDataPoint(point);
        }

        // Добавляем точку на график
        updateChart(point);
    }

    qDebug() << "History loaded successfully";

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

    //static int pointIndex = 0; // добавляем счетчик точек

    double value = point.value(); // Без нормализации


        qDebug() << "updateChart called:" << point.type() << "value=" << value;
        //Оставляем сигнал для QML
        emit chartDataReceived(m_pointIndex, value, point.type());
        m_pointIndex++; // 0, 1, 2, 3...
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

void MainController::setCity(const QString& city)
{
    qDebug() << "setCity called:" << city;

    if (m_weatherFetcher) {
        // Проверяем, что выбран реальный город
        bool validCity = !city.isEmpty() && city != "Select City" && city != "▼ Select City";

        if (validCity) {
            m_weatherFetcher->setCurrentCity(city);
            if (!m_citySelected) {
                m_citySelected = true;
                emit citySelectedChanged();
                qDebug() << "City selected:" << city;
            }

            // Если погода уже запущена - обновляем данные для нового города
            if (m_weatherRunning) {
                m_weatherFetcher->stopFetching();
                m_weatherFetcher->fetchNow(city);
                m_weatherFetcher->startFetching(300);
                qDebug() << "Weather automatically update for city:" << city;
            }
        } else {
            if (m_citySelected) {
                m_citySelected = false;
                emit citySelectedChanged();
                qDebug() << "City deselected";
            }
        }
    }
}
void MainController::clearData()//метод для очистки данных(графика)
{
    //Очищаем модель данных (таблицу)
    m_dataModel->clear();
    m_pointIndex = 0;

    //Отправляем сигнал для очистки графика в QML
    emit clearGraphRequested();
}

// Метод добавления свечей
void MainController::addCandle(double open, double high, double low, double close, const QString& timestamp)
{
    CandleData candle;
    candle.open = open;
    candle.high = high;
    candle.low = low;
    candle.close = close;
    candle.timestamp = QDateTime::fromString(timestamp, Qt::ISODate);

    m_candles.append(candle);

    // Отправить в сигнал QML
    emit candleDataReceived(open, high, low, close, timestamp);

        qDebug() << "Candle added:" << timestamp << "O:" << open << "H:" << high << "L:" << low << "C:" << close;
}

// В MainController.cpp добавим тестовый метод:
/*void MainController::generateTestCandles()
{
    // Имитация 20 свечей
    QDateTime time = QDateTime::currentDateTime();
    double price = 50000.0;  // Начальная цена BTC/USDT

    for (int i = 0; i < 20; i++) {
        double change = (rand() % 200 - 100) / 100.0;  // -1% до +1%
        double open = price;
        double close = price * (1 + change);
        double high = qMax(open, close) + (rand() % 100);
        double low = qMin(open, close) - (rand() % 100);

        addCandle(open, high, low, close, time.toString(Qt::ISODate));

        price = close;
        time = time.addSecs(3600);  // +1 час
    }
*/