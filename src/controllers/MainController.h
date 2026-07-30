#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "../models/DataModel.h"
#include "../network/WebSocketServer.h"
#include "../database/DatabaseManager.h"
#include "../core/DataProcessor.h"
#include "../network/WeatherFetcher.h"
#include "../export/ReportExporter.h"
#include "../network/ExchangeClient.h"
#include "../models/CandleModel.h"
#include "../chart/TradingChartManager.h"

/**
 * Главный контроллер приложения
 * Прием данных через WebSocket
 * Обработка данных (DataProcessor)
 * Сохранение в PostgreSQL (DatabaseManager)
 * Отображение в QML (DataModel)
 * Автоматический сбор погоды (WeatherFetcher)
 * Работа с биржей (ExchangeClient)
 */
class MainController : public QObject
{
    Q_OBJECT

    // Q_PROPERTY для QML
    Q_PROPERTY(TradingChartManager* chartManager READ chartManager CONSTANT)
    Q_PROPERTY(DataModel* dataModel READ dataModel CONSTANT)
    Q_PROPERTY(CandleModel* candleModel READ candleModel CONSTANT)

    Q_PROPERTY(bool isServerRunning READ isServerRunning NOTIFY serverRunningChanged)
    Q_PROPERTY(bool isWeatherRunning READ isWeatherRunning NOTIFY weatherRunningChanged)
    Q_PROPERTY(bool isCitySelected READ isCitySelected NOTIFY citySelectedChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool isRealtimeConnected READ isRealtimeConnected NOTIFY realtimeConnectedChanged)

    // Информационная панель биржи
    Q_PROPERTY(QString currentPrice READ currentPrice NOTIFY currentPriceChanged)
    Q_PROPERTY(QString previousPrice READ previousPrice NOTIFY previousPriceChanged)
    Q_PROPERTY(QString priceChange READ priceChange NOTIFY priceChangeChanged)
    Q_PROPERTY(QString priceChangePercent READ priceChangePercent NOTIFY priceChangePercentChanged)
    Q_PROPERTY(QString highPrice READ highPrice NOTIFY highPriceChanged)
    Q_PROPERTY(QString lowPrice READ lowPrice NOTIFY lowPriceChanged)
    Q_PROPERTY(QString volume READ volume NOTIFY volumeChanged)
    Q_PROPERTY(QString lastUpdateTime READ lastUpdateTime NOTIFY lastUpdateTimeChanged)

    // Информационная панель погоды
    Q_PROPERTY(QString weatherCity READ weatherCity NOTIFY weatherCityChanged)
    Q_PROPERTY(QString weatherDescription READ weatherDescription NOTIFY weatherDescriptionChanged)

public:
    explicit MainController(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    ~MainController();

    // Геттеры для QML
    DataModel* dataModel() const { return m_dataModel; }
    bool isServerRunning() const { return m_serverRunning; }
    bool isWeatherRunning() const { return m_weatherRunning; }
    bool isCitySelected() const { return m_citySelected; }
    bool isLoading() const { return m_isLoadingCandles; }
    bool isRealtimeConnected() const { return m_exchangeClient && m_exchangeClient->isRealtimeConnected(); }
    TradingChartManager* chartManager() const { return m_chartManager; }
    CandleModel* candleModel() const { return m_candleModel; }

    // Геттеры для биржи
    QString currentPrice() const { return m_currentPrice; }
    QString previousPrice() const { return m_previousPrice; }
    QString priceChange() const { return m_priceChange; }
    QString priceChangePercent() const { return m_priceChangePercent; }
    QString highPrice() const { return m_highPrice; }
    QString lowPrice() const { return m_lowPrice; }
    QString volume() const { return m_volume; }
    QString lastUpdateTime() const { return m_lastUpdateTime; }

    // Геттеры для погоды
    QString weatherCity() const { return m_weatherCity; }
    QString weatherDescription() const { return m_weatherDescription; }


    // Q_INVOKABLE МЕТОДЫ ДЛЯ QML


    // Сервер
    Q_INVOKABLE bool startServer(quint16 port = 8080);
    Q_INVOKABLE void stopServer();

    // Данные
    Q_INVOKABLE void loadHistory(const QDateTime& from, const QDateTime& to);
    Q_INVOKABLE void clearData();

    // Экспорт
    Q_INVOKABLE void exportToCSV();
    Q_INVOKABLE void exportToPDF();

    // Погода
    Q_INVOKABLE void startWeather();
    Q_INVOKABLE void stopWeather();
    Q_INVOKABLE void setCity(const QString& city);

    // Биржа (НОВЫЙ API)
    Q_INVOKABLE void loadCandles(const QString& symbol, int intervalIndex, int limit = 100);

    // Устаревший метод (оставлен для совместимости, но не используется)
    Q_INVOKABLE void addCandle(double open, double high, double low, double close, const QString& timestamp);

    Q_INVOKABLE void startRealtime(); // Явное подключение WebSocket к уже загруженному рынку
    Q_INVOKABLE void stopRealtime();  // Явное отключение WebSocket

signals:
    // Сервер
    void serverRunningChanged();

    // График
    void chartDataReceived(qreal timestamp, qreal value, QString type);
    void clearGraphRequested();

    // Погода
    void weatherRunningChanged();
    void citySelectedChanged();
    void weatherCityChanged();
    void weatherDescriptionChanged();

    // Биржа
    void candlesUpdated();
    void isLoadingChanged();
    void realtimeConnectedChanged();

    // Информационная панель биржи
    void currentPriceChanged();
    void previousPriceChanged();
    void priceChangeChanged();
    void priceChangePercentChanged();
    void highPriceChanged();
    void lowPriceChanged();
    void volumeChanged();
    void lastUpdateTimeChanged();

private slots:
    // WebSocket сервер
    void onDataReceived(const QString& data);
    void onDataProcessed(const DataPoint& point);
    void updateChart(const DataPoint& point);

    // Погода
    void onWeatherDataReceived(const WeatherData& data);

    // Биржа (НОВЫЕ СЛОТЫ)
    void onCandlesLoaded(const QList<CandleData>& candles);
    void onNewCandleTick(const CandleData& candle);
    void onExchangeError(const QString& error);

private:
    // Вспомогательные методы
    DataPoint parseData(const QString& data);
    void updateTickerInfo(const CandleData& candle);


    // КОМПОНЕНТЫ

    QQmlApplicationEngine* m_engine;

    // Основные компоненты
    DataModel* m_dataModel;
    WebSocketServer* m_server;
    DatabaseManager* m_database;
    DataProcessor* m_processor;
    WeatherFetcher* m_weatherFetcher;
    ReportExporter* m_exporter;

    // Биржа
    ExchangeClient* m_exchangeClient;
    CandleModel* m_candleModel;
    TradingChartManager* m_chartManager;


    // СОСТОЯНИЯ


    // Сервер
    bool m_serverRunning = false;

    // Погода
    bool m_weatherRunning = false;
    bool m_citySelected = false;
    int m_weatherIndex = 0;

    // Биржа
    bool m_isLoadingCandles = false;
    int m_pointIndex = 0;

    // Параметры загрузки (для onCandlesLoaded)
    QString m_pendingSymbol;
    int m_pendingIntervalIndex = 0;
    int m_pendingLimit = 100;

    // Список свечей (для совместимости)
    QList<CandleData> m_candles;


    // ДАННЫЕ ДЛЯ ИНФОРМАЦИОННЫХ ПАНЕЛЕЙ

    // Биржа
    QString m_currentPrice;
    QString m_previousPrice;
    QString m_priceChange;
    QString m_priceChangePercent;
    QString m_highPrice;
    QString m_lowPrice;
    QString m_volume;
    QString m_lastUpdateTime;

    // Погода
    QString m_weatherCity;
    QString m_weatherDescription;
};

#endif // MAINCONTROLLER_H