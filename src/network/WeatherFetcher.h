#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <qobject.h>
#include <QNetworkAccessManager> //Для HTTP-запросов к API
#include <QNetworkReply>         //Для обработки ответов от API
#include <QTimer>                //Для переодических запросов

class WeatherFetcher : public QObject //Класс для получения данных о погоде через OpenWeatherMap API

{
    Q_OBJECT //Макрос Qt для работы с сигналами/слотами
public:

    //Конструктор (для автоматического удаления)
    explicit WeatherFetcher(QObject *parent = nullptr);
    ~WeatherFetcher();//Деструктор (останавливает таймер и освобождает ресурсы)

    void setApiKey(const QString& key);//Устанавливает API ключ OpenWeatherMap
    void startFetching(int intervalSecond = 600);//Запускает периодический сбор данных о погоде(интервал 600 = 10мин)
    void stopFetching();//Останавливает периодический сбор данных
    void fetchNow(const QString& city = "Moscow");//Принудительный запрос погоды прямо сейчас(город по умолчанию "Moscow")


signals:
        /** Сигнал, испускаемый при получении новых данных о погоде
           Для каждого параметра испускается отдельный сигнал
         */
    void weatherDataReceived(const QString& type, double value, const QString& unit);
    void errorOccurred(const QString& error); //Сигнал об ошибке

private slots:
    void onReplyFinished(QNetworkReply* reply); //Обработчик ответа от OpenWeatherMap API
    void onTimerTimeout(); //Обработчик таймера

private:
    QNetworkAccessManager* m_manager; //менеджер http-запросов
    QTimer* m_timer; // Таймер для переодических запросов
    QString m_apiKey; // API ключ OpenWeqtherMap
    bool m_isRunning; // Флаг активности (запущен/остановлен)
    QString m_currentCity; // Текущий город для запроса

    void fetchWeather(const QString& city);//Отправляет HTTP-запрос к OpenWeatherMap API
    void parseResponse(const QByteArray& data); //Парсит JSON-ответ от API


};

#endif // WEATHERFETCHER_H
