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
    , m_exporter(new ReportExporter(this))
    , m_weatherFetcher(new WeatherFetcher(this))
    , m_pointIndex(0)
    , m_weatherIndex(0)
    , m_exchangeClient(new ExchangeClient(this))
    , m_candleModel(new CandleModel(this))
    , m_chartManager(new TradingChartManager(this))
{
    // Контекстные свойства для QML
    m_engine->rootContext()->setContextProperty("controller", this);
    m_engine->rootContext()->setContextProperty("candleModel", m_candleModel);
    m_engine->rootContext()->setContextProperty("chartManager", m_chartManager);


    // ПОДКЛЮЧЕНИЕ СИГНАЛОВ (ОДИН РАЗ)


    // WebSocket сервер
    connect(m_server, &WebSocketServer::dataReceived, this, &MainController::onDataReceived);

    // Обработка данных
    connect(m_processor, &DataProcessor::dataProcessed, this, &MainController::updateChart);
    connect(m_processor, &DataProcessor::dataProcessed, this, &MainController::onDataProcessed);
    connect(m_processor, &DataProcessor::dataProcessed, m_dataModel, &DataModel::addDataPoint);

    // Экспорт
    connect(m_exporter, &ReportExporter::exportFinished,
            [](bool success, const QString& filePath, const QString& error) {
                if (success) {
                    qDebug() << "Export successful:" << filePath;
                } else {
                    qDebug() << "Export failed:" << error;
                }
            });


    // BINANCE EXCHANGE (НОВЫЙ API)

    // Загрузка истории
    connect(m_exchangeClient, &ExchangeClient::candlesLoaded,
            this, &MainController::onCandlesLoaded);

    // Новые свечи в реальном времени
    connect(m_exchangeClient, &ExchangeClient::newCandleTick,
            this, &MainController::onNewCandleTick);

    // Ошибки
    connect(m_exchangeClient, &ExchangeClient::errorOccurred,
            this, &MainController::onExchangeError);

    // Статус подключения
    connect(m_exchangeClient, &ExchangeClient::connectionStatusChanged,
            this, [](bool connected) {
                qDebug() << "Exchange connection status:" << (connected ? "Connected" : "Disconnected");
            });


    // ПОГОДА


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

    connect(m_weatherFetcher, &WeatherFetcher::weatherDataReceived,
            this, &MainController::onWeatherDataReceived);
    connect(m_weatherFetcher, &WeatherFetcher::errorOccurred,
            [](const QString& error) { qDebug() << "Weather error:" << error; });


    // БАЗА ДАННЫХ


    QString dbPassword = SecretManager::getDbPassword();
    if (dbPassword.isEmpty()) {
        qDebug() << "WARNING: Database password not found in secure storage!";
        dbPassword = "";
    }

    const QString DB_HOST = "localhost";
    const int DB_PORT = 5432;
    const QString DB_NAME = "datamonitor";
    const QString DB_USER = "postgres";

    if (!m_database->connectToPostgreSQL(DB_HOST, DB_PORT, DB_NAME, DB_USER, dbPassword)) {
        qDebug() << "Failed to connect to PostgreSQL";
        qDebug() << "Please ensure PostgreSQL is running and database 'datamonitor' exists";
    } else {
        qDebug() << "PostgreSQL connected successfully";
    }


    // QML РЕГИСТРАЦИЯ

    qmlRegisterUncreatableType<DataModel>("com.datamonitor", 1, 0, "DataModel", "Cannot create DataModel in QML");
    qmlRegisterUncreatableType<CandleModel>("com.datamonitor", 1, 0, "CandleModel", "Cannot create CandleModel in QML");
}


// ДЕСТРУКТОР

MainController::~MainController()
{
    qDebug() << "=== MainController destructor START ===";

    if (m_exchangeClient) {
        // closeRealtime() вызывается в деструкторе ExchangeClient
        // но мы можем явно остановить
    }

    if (m_weatherFetcher) {
        m_weatherFetcher->stopFetching();
    }

    stopServer();

    qDebug() << "=== MainController destructor END ===";
}


// СЕРВЕР

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


// ОБРАБОТКА ДАННЫХ

void MainController::onDataReceived(const QString& data)
{
    qDebug() << "Received data:" << data;
    DataPoint point = parseData(data);
    if (point.isValid()) {
        m_processor->processDataPoint(point);
        if (!m_database->saveDataPoint(point)) {
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
        qDebug() << "Database is null!";
        return;
    }

    QList<DataPoint> history = m_database->loadDataPoints(from, to);
    qDebug() << "Loaded" << history.size() << "points from database";

    if (history.isEmpty()) {
        qDebug() << "History is empty";
        return;
    }

    if (m_dataModel) {
        m_dataModel->clear();
    }
    m_pointIndex = 0;
    emit clearGraphRequested();

    for (const DataPoint& point : history) {
        if (m_dataModel) {
            m_dataModel->addDataPoint(point);
        }
        updateChart(point);
    }

    qDebug() << "History loaded successfully";
}

DataPoint MainController::parseData(const QString& data)
{
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
    double value = point.value();
    qDebug() << "updateChart called:" << point.type() << "value=" << value;
    emit chartDataReceived(m_pointIndex, value, point.type());
    m_pointIndex++;
}


// ПОГОДА

void MainController::onWeatherDataReceived(const WeatherData& data)
{
    QList<DataPoint> points = {
                               { data.timestamp, "temperature", data.temperature, "°C"},
                               { data.timestamp, "pressure", data.pressure, "hPa"},
                               { data.timestamp, "humidity", data.humidity, "%"},
                               };

    for (const auto& point : points) {
        m_database->saveDataPoint(point);
        m_dataModel->addDataPoint(point);
    }

    m_weatherDescription = data.description;
    emit weatherDescriptionChanged();

    emit chartDataReceived(m_weatherIndex, data.temperature, "temperature");
    emit chartDataReceived(m_weatherIndex, data.pressure, "pressure");
    emit chartDataReceived(m_weatherIndex, data.humidity, "humidity");

    m_weatherIndex++;
    qDebug() << "Weather measurement #" << (m_weatherIndex - 1)
             << "T=" << data.temperature
             << "P=" << data.pressure
             << "H=" << data.humidity;
}

void MainController::startWeather()
{
    if (m_weatherFetcher) {
        m_weatherFetcher->startFetching(300);
        m_weatherRunning = true;
        emit weatherRunningChanged();
        qDebug() << "Weather monitoring started (interval: 300 sec)";
    }
}

void MainController::stopWeather()
{
    if (m_weatherFetcher) {
        m_weatherFetcher->stopFetching();
        m_weatherRunning = false;
        emit weatherRunningChanged();
        qDebug() << "Weather monitoring stopped";
    }
}

void MainController::setCity(const QString& city)
{
    qDebug() << "setCity called:" << city;

    if (m_weatherFetcher) {
        bool validCity = !city.isEmpty() && city != "Select City" && city != "▼ Select City";

        if (validCity) {
            m_weatherCity = city;
            emit weatherCityChanged();

            m_weatherFetcher->setCurrentCity(city);
            if (!m_citySelected) {
                m_citySelected = true;
                emit citySelectedChanged();
                qDebug() << "City selected:" << city;
            }

            m_dataModel->clear();
            m_weatherIndex = 0;
            emit clearGraphRequested();

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


// ЭКСПОРТ

void MainController::exportToCSV()
{
    QString fileName = QString("DataMonitor_Export_%1.csv")
    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                       + "/" + fileName;

    m_exporter->exportCurrentData(m_dataModel, filePath, "csv");
}

void MainController::exportToPDF()
{
    QString fileName = QString("DataMonitor_Report_%1.pdf")
    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                       + "/" + fileName;

    m_exporter->exportCurrentData(m_dataModel, filePath, "pdf");
}


// ОЧИСТКА ДАННЫХ
void MainController::clearData()
{
    m_dataModel->clear();
    m_pointIndex = 0;
    m_weatherIndex = 0;
    emit clearGraphRequested();
}


// БИРЖА (НОВЫЙ API)
void MainController::loadCandles(const QString& symbol, int intervalIndex, int limit)
{
    if (!m_exchangeClient) return;

    if (m_isLoadingCandles) {
        qDebug() << "Already loading, ignoring...";
        return;
    }

    m_isLoadingCandles = true;
    emit isLoadingChanged();

    Interval interval = static_cast<Interval>(intervalIndex);
    QString useSymbol = symbol.isEmpty() ? "BTCUSDT" : symbol;

    m_pendingSymbol = useSymbol;
    m_pendingIntervalIndex = intervalIndex;
    m_pendingLimit = limit;

    // Очищаем старые данные
    m_candleModel->clear();
    m_chartManager->clearSeries();
    emit clearGraphRequested();

    // Единый метод загрузки рынка
    m_exchangeClient->loadMarket(useSymbol, interval, limit);

    qDebug() << "Loading market:" << useSymbol << "interval:" << intervalIndex << "limit:" << limit;
}

void MainController::onCandlesLoaded(const QList<CandleData>& candles)
{
    if (!m_isLoadingCandles) {
        qDebug() << "Stale candlesLoaded, ignoring";
        return;
    }

    m_candleModel->setCandles(candles);
    m_chartManager->updateSeries(m_candleModel);

    if (!candles.isEmpty()) {
        updateTickerInfo(candles.last());
    }

    emit candlesUpdated();

    m_isLoadingCandles = false;
    emit isLoadingChanged();

    qDebug() << "History loaded:" << candles.size() << "candles, WebSocket will be opened by ExchangeClient";
}

void MainController::onNewCandleTick(const CandleData& candle)
{
    if (m_isLoadingCandles) {
        qDebug() << "Ignoring candle during load";
        return;
    }

    if (!m_exchangeClient->isRealtimeConnected()) {
        qDebug() << "Ignoring candle - not connected";
        return;
    }

    m_candleModel->addOrUpdateCandle(candle);
    m_chartManager->updateLastCandle(candle);
    updateTickerInfo(candle);
    emit candlesUpdated();
}

void MainController::onExchangeError(const QString& error)
{
    qDebug() << "Exchange error:" << error;

    if (m_isLoadingCandles) {
        m_isLoadingCandles = false;
        emit isLoadingChanged();
    }
}


// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
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
    qDebug() << "Candle added:" << timestamp << "O:" << open << "H:" << high << "L:" << low << "C:" << close;
}

void MainController::startRealtimeCandles(const QString& symbol, int intervalIndex)
{
    // Этот метод устарел. Теперь всё управление через loadCandles()
    // который вызывает ExchangeClient::loadMarket()
    qDebug() << "startRealtimeCandles is deprecated."
             << "Use loadCandles(" << symbol << "," << intervalIndex << ") instead.";

    // Просто вызываем loadCandles с тем же символом и интервалом
    // и стандартным лимитом 100
    loadCandles(symbol, intervalIndex, 100);
}

void MainController::stopRealtimeCandles()
{
    // Теперь управление через ExchangeClient::loadMarket
    // Этот метод можно удалить или оставить как заглушку
    qDebug() << "stopRealtimeCandles is deprecated - use loadCandles with new symbol";
}

void MainController::updateTickerInfo(const CandleData& candle)
{
    if (!m_currentPrice.isEmpty()) {
        m_previousPrice = m_currentPrice;
        emit previousPriceChanged();
    }

    m_currentPrice = QString::number(candle.close, 'f', 2);
    emit currentPriceChanged();

    if (m_highPrice.isEmpty() || candle.high > m_highPrice.toDouble()) {
        m_highPrice = QString::number(candle.high, 'f', 2);
        emit highPriceChanged();
    }
    if (m_lowPrice.isEmpty() || candle.low < m_lowPrice.toDouble()) {
        m_lowPrice = QString::number(candle.low, 'f', 2);
        emit lowPriceChanged();
    }

    m_volume = QString::number(candle.volume, 'f', 2);
    emit volumeChanged();

    m_lastUpdateTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    emit lastUpdateTimeChanged();

    if (!m_previousPrice.isEmpty()) {
        double prev = m_previousPrice.toDouble();
        double curr = candle.close;
        double change = curr - prev;
        double changePercent = (change / prev) * 100.0;

        m_priceChange = QString::number(change, 'f', 2);
        m_priceChangePercent = QString::number(changePercent, 'f', 2) + "%";

        emit priceChangeChanged();
        emit priceChangePercentChanged();
    }
}