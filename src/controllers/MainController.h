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
    Q_PROPERTY(DataModel* dataModel READ dataModel CONSTANT) // Модель данных для таблицы
    Q_PROPERTY(bool isServerRunning READ isServerRunning NOTIFY serverRunningChanged) // Статус сервера

public:
    //Конструктор
    explicit MainController(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    //Деструктор(Останавливает сервер и освобождает ресурсы)
    ~MainController();
    
    DataModel* dataModel() const { return m_dataModel; } //Возвращает модель данных для QML
    bool isServerRunning() const { return m_serverRunning; } //Возвращает статус сервера (запущен/остановлен)
    
    Q_INVOKABLE bool startServer(quint16 port = 8080); //Запускает WebSocket-сервер
    Q_INVOKABLE void stopServer(); //Останавливает WebSocket-сервер
    Q_INVOKABLE void loadHistory(const QDateTime& from, const QDateTime& to); //Загружает историю данных за указанный период
    
signals:
    void serverRunningChanged(); //Сигнал об изменении статуса сервера
    void chartDataReceived(qreal timestamp, qreal value); //Сигнал для передачи данных в график

private slots:
    void onDataReceived(const QString& data); //Обработчик получения данных от WebSocket-сервера
    void onDataProcessed(const DataPoint& point);//Обработчик обработанной точки данных
    void updateChart(const DataPoint& point);//Обновляет график новыми данными
    void onWeatherDataReceived(const QString& type, double value, const QString& unit);//Обработчик данных о погоде от WeatherFetcher

private:
    QQmlApplicationEngine* m_engine;//Движок QML (для регистрации контроллера)
    DataModel* m_dataModel;//Модель данных для таблицы
    WebSocketServer* m_server;// WebSocket-сервер
    DatabaseManager* m_database;//Менеджер базы данных PostgreSQL
    DataProcessor* m_processor;//Обработчик данных
    bool m_serverRunning = false;//Флаг статуса сервера
    WeatherFetcher* m_weatherFetcher;//Получатель погоды
    
    DataPoint parseData(const QString& data);//Парсит JSON-строку в объект DataPoint
};

#endif // MAINCONTROLLER_H