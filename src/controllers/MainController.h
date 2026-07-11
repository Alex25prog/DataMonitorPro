#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject> //Базовый класс для объектов Qt с сигналами/слотами
#include <QQmlApplicationEngine> //Движок для загрузки QML-интерфейса
#include <QQmlContext> // Контекст для передачи объектов в QML

#include "../models/DataModel.h" //Модель данных для таблицы (QAbstractListModel)
#include "../network/WebSocketServer.h" //WebSocket-сервер для приема данных
#include "../database/DatabaseManager.h" //Менеджер базы данных (PostgreSQL)
#include "../core/DataProcessor.h" // Обработчик данных (валидация, статистика)
#include "../network/WeatherFetcher.h" //Получатель погоды из OpenWeatherMap API
#include "../export/ReportExporter.h"
#include "../network/ExchangeClient.h"
#include "../models/CandleModel.h"
#include "../chart/TradingChartManager.h"

/**
 Главный контроллер приложения
 Прием данных через WebSocket
 Обработка данных (DataProcessor)
 Сохранение в PostgreSQL (DatabaseManager)
 Отображение в QML (DataModel)
 Автоматический сбор погоды (WeatherFetcher)
*/


class MainController : public QObject
{
    Q_OBJECT // Макрос для поддержки сигналов/слотов и метаобъектной системы Qt
    // Q_PROPERTY позволяет QML обращаться к этим свойствам как к обычным переменным
    Q_PROPERTY(TradingChartManager* chartManager READ chartManager CONSTANT)
    Q_PROPERTY(DataModel* dataModel READ dataModel CONSTANT) // Модель данных для таблицы
    Q_PROPERTY(bool isServerRunning READ isServerRunning NOTIFY serverRunningChanged) // Статус сервера
    Q_PROPERTY(bool isWeatherRunning READ isWeatherRunning NOTIFY weatherRunningChanged)
    Q_PROPERTY(bool isCitySelected READ isCitySelected NOTIFY citySelectedChanged)

public:
    //Конструктор
    explicit MainController(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    //Деструктор(Останавливает сервер и освобождает ресурсы)
    ~MainController();
    
    DataModel* dataModel() const { return m_dataModel; } //Возвращает модель данных для QML
    bool isServerRunning() const { return m_serverRunning; } //Возвращает статус сервера (запущен/остановлен)
    bool isWeatherRunning() const { return m_weatherRunning; }
    bool isCitySelected() const { return m_citySelected; }
    TradingChartManager* chartManager() const { return m_chartManager; }
    
    Q_INVOKABLE bool startServer(quint16 port = 8080); //Запускает WebSocket-сервер
    Q_INVOKABLE void stopServer(); //Останавливает WebSocket-сервер
    Q_INVOKABLE void loadHistory(const QDateTime& from, const QDateTime& to); //Загружает историю данных за указанный период
    Q_INVOKABLE void exportToCSV();//методы экспорта
    Q_INVOKABLE void exportToPDF();//методы экспорта
    Q_INVOKABLE void startWeather();
    Q_INVOKABLE void stopWeather();
    Q_INVOKABLE void clearData();
    Q_INVOKABLE void setCity(const QString& city);//Для приема города
    Q_INVOKABLE void addCandle(double open, double high, double low, double close, const QString& timestamp);
    Q_INVOKABLE void loadCandles(const QString& symbol, int intervalIndex, int limit = 100);
    Q_INVOKABLE void startRealtimeCandles(const QString& symbol, int intervalIndex);
    Q_INVOKABLE void stopRealtimeCandles();
    Q_PROPERTY(CandleModel* candleModel READ candleModel CONSTANT)
    CandleModel* candleModel() const { return m_candleModel; }

signals:
    void serverRunningChanged(); //Сигнал об изменении статуса сервера
    void chartDataReceived(qreal timestamp, qreal value, QString type); //Сигнал для передачи данных в график
    void weatherRunningChanged();
    void citySelectedChanged();
    void clearGraphRequested();//Сигнал для очистки графика
    //void candleDataReceived(const QList<CandleData>& candles);
    //void weatherMeasurementReceived(int index, double temperature, double pressure, double humidity);
    //void newCandle(const CandleData& candle);
    void candlesUpdated();

private slots:
    void onDataReceived(const QString& data); //Обработчик получения данных от WebSocket-сервера
    void onDataProcessed(const DataPoint& point);//Обработчик обработанной точки данных
    void updateChart(const DataPoint& point);//Обновляет график новыми данными
    void onWeatherDataReceived(const WeatherData& data);//Обработчик данных о погоде от WeatherFetcher

private:
    QQmlApplicationEngine* m_engine;//Движок QML (для регистрации контроллера)
    DataModel* m_dataModel;//Модель данных для таблицы
    WebSocketServer* m_server;// WebSocket-сервер
    DatabaseManager* m_database;//Менеджер базы данных PostgreSQL
    DataProcessor* m_processor;//Обработчик данных
    bool m_serverRunning = false;//Флаг статуса сервера
    WeatherFetcher* m_weatherFetcher;//Получатель погоды
    ReportExporter* m_exporter;
    DataPoint parseData(const QString& data);//Парсит JSON-строку в объект DataPoint
    bool m_weatherRunning = false;
    bool m_citySelected = false;
    QList<CandleData> m_candles;
    int m_pointIndex = 0;
    int m_weatherIndex = 0; // Счетчик измерений погоды
    ExchangeClient* m_exchangeClient = nullptr;
    CandleModel* m_candleModel = nullptr;
    TradingChartManager* m_chartManager = nullptr;

};

#endif // MAINCONTROLLER_H