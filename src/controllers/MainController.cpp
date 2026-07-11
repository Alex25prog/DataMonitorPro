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
    , m_pointIndex(0)
    , m_weatherIndex(0)
    , m_exchangeClient(new ExchangeClient(this))
    , m_candleModel(new CandleModel(this))
    , m_chartManager(new TradingChartManager(this))

{
    // Добавим контекстные свойства
     m_engine->rootContext()->setContextProperty("candleModel", m_candleModel);

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

    // Подключаем сигналы биржи
    connect(m_exchangeClient, &ExchangeClient::candlesLoaded,
            [this](const QList<CandleData>& candles) {
                m_candleModel->setCandles(candles);
                qDebug() << "Candles loaded:" << candles.size();
                // Обновляем график через ChartManager
                m_chartManager->updateSeries(m_candleModel);
                // Сигнал для QML что данные обновлены
                emit candlesUpdated();
    });

    connect(m_exchangeClient, &ExchangeClient::newCandle,
            [this](const CandleData& candle) {
                m_candleModel->addCandle(candle);
                qDebug() << "New candle added:" << candle.openTime;
                // Обновляем график через ChartManager
                m_chartManager->updateSeries(m_candleModel);
                // Сигнал для QML
                emit candlesUpdated();

    });

    connect(m_exchangeClient, &ExchangeClient::errorOccurred,
            [](const QString& error) {
                qDebug() << "Exchange error:" << error;
    });

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

    qmlRegisterUncreatableType<CandleModel>("com.datamonitor", 1,0, "CandleModel", "Cannot create CandleModel in QML");
    // Делаем контроллер доступным из QML
    m_engine->rootContext()->setContextProperty("controller", this);
}

//Деструктор
MainController::~MainController()
{
    qDebug() << "===MainController destructor SRART ===";

    if (m_exchangeClient) {
        m_exchangeClient->stopRealtimeUpdates();
    }

    if (m_weatherFetcher) {
        m_weatherFetcher->stopFetching();
    }

    stopServer();

    qDebug() << "===MainCintroller destructor END ===";

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

void MainController::onWeatherDataReceived(const WeatherData& data)
{
    // Создаем точки для модели и БДэ
    QList<DataPoint> points = {
        { data.timestamp, "temperature", data.temperature, "°C"},
        { data.timestamp, "pressure", data.pressure, "hPa"},
        { data.timestamp, "humidity", data.humidity, "%"},

    };

    // Сохраняем в базу и модель
    for (const auto& point : points) {
        m_database->saveDataPoint(point);
        m_dataModel->addDataPoint(point);
    }

    // Отправляем три точки с ОДНИМ индексом
    emit chartDataReceived(m_weatherIndex, data.temperature, "temperature");
    emit chartDataReceived(m_weatherIndex, data.pressure, "pressure");
    emit chartDataReceived(m_weatherIndex, data.humidity, "humidity");

    // Увеличиваем индекс для следующего измерения
    m_weatherIndex++;
    qDebug() << "Weather measurement #" << (m_weatherIndex -1)
             << "T=" << data.temperature
             << "P=" << data.pressure
             << "H=" << data.humidity;
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

            // Очищаем график при смене города
            m_dataModel->clear();
            m_weatherIndex = 0;
            emit clearGraphRequested();

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
    m_weatherIndex = 0;

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
    candle.openTime = QDateTime::fromString(timestamp, Qt::ISODate).toMSecsSinceEpoch();
    candle.closeTime = candle.openTime + 3600000;
    candle.volume = 0;
    candle.isClosed = true;

    m_candles.append(candle);

    // Отправить в сигнал QML
   // emit candleDataReceived(open, high, low, close, timestamp);

        qDebug() << "Candle added:" << timestamp << "O:" << open << "H:" << high << "L:" << low << "C:" << close;
}

// Методы для работы с биржей

void MainController::loadCandles(const QString& symbol, int intervalIndex, int limit)
{
    if (m_exchangeClient) {
        Interval interval = static_cast<Interval>(intervalIndex);
        m_exchangeClient->loadHistory(symbol, interval, limit);
        qDebug() << "Loading candles for" << symbol << "limit:" << limit;
    }
}

void MainController::startRealtimeCandles(const QString& symbol, int intervalIndex)
{
    if (m_exchangeClient) {
        Interval interval = static_cast<Interval>(intervalIndex);
        m_exchangeClient->startRealtimeUpdates(symbol, interval);
        qDebug() << "Realtime candles started for" << symbol;
    }
}

void MainController::stopRealtimeCandles()
{
    if (m_exchangeClient) {
        m_exchangeClient->stopRealtimeUpdates();
        qDebug() << "Realtime candles stopped";
    }
}

