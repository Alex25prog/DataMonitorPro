#ifndef WEATHERFETCHER_CPP
#define WEATHERFETCHER_CPP

#include "WeatherFetcher.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

WeatherFetcher::WeatherFetcher(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_timer(new QTimer(this))
    , m_isRunning(false)
    , m_currentCity("Moscow")
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &WeatherFetcher::onReplyFinished);
    connect(m_timer, &QTimer::timeout,
            this, &WeatherFetcher::onTimerTimeout);
}

WeatherFetcher::~WeatherFetcher()
{
    stopFetching();
}

void WeatherFetcher::setApiKey(const QString& key)
{
    m_apiKey = key;
    qDebug() << "OpenWeatherMap API key set";
}

void WeatherFetcher::startFetching(int intervalSeconds)
{
    if (m_isRunning) {
        qDebug() << "WeatherFetcher already running";
        return;
    }

    if (m_apiKey.isEmpty()) {
        qDebug() << "WeatherFetcher: API key not set";
        emit errorOccurred("API key not configured");
        return;
    }

    m_isRunning = true;
    m_timer->start(intervalSeconds * 1000);
    fetchNow(m_currentCity);

    qDebug() << "WeatherFetcher started (interval:" << intervalSeconds << "sec)";
}

void WeatherFetcher::stopFetching()
{
    if (!m_isRunning) return;
    m_timer->stop();
    m_isRunning = false;
    qDebug() << "WeatherFetcher stopped";
}

void WeatherFetcher::fetchNow(const QString& city)
{
    if (!m_isRunning) return;
    m_currentCity = city;
    fetchWeather(city);
}

void WeatherFetcher::fetchWeather(const QString& city)
{
    if (m_apiKey.isEmpty()) {
        qDebug() << "Weather API key not set";
        return;
    }

    QString url = QString("https://api.openweathermap.org/data/2.5/weather"
                          "?q=%1&appid=%2&units=metric")
                      .arg(city).arg(m_apiKey);

    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "DataMonitorPro/1.0");

    qDebug() << "Fetching weather for" << city;
    m_manager->get(request);
}

void WeatherFetcher::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Weather API error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    parseResponse(data);

    reply->deleteLater();
}

void WeatherFetcher::parseResponse(const QByteArray& data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Invalid JSON from Weather API";
        emit errorOccurred("Invalid JSON response");
        return;
    }

    QJsonObject root = doc.object();

    if (!root.contains("main")) {
        qDebug() << "Unexpected weather API response";
        if (root.contains("message")) {
            qDebug() << "API message:" << root["message"].toString();
        }
        return;
    }

    QJsonObject main = root["main"].toObject();

    // Температура
    double temp = main["temp"].toDouble();
    emit weatherDataReceived("temperature", temp, "°C");
    qDebug() << "Temperature:" << temp << "°C";

    // Давление
    double pressure = main["pressure"].toDouble();
    emit weatherDataReceived("pressure", pressure, "hPa");
    qDebug() << "Pressure:" << pressure << "hPa";

    // Влажность
    double humidity = main["humidity"].toDouble();
    emit weatherDataReceived("humidity", humidity, "%");
    qDebug() << "Humidity:" << humidity << "%";

    // Описание погоды (опционально)
    if (root.contains("weather")) {
        QJsonArray weather = root["weather"].toArray();
        if (!weather.isEmpty()) {
            QString description = weather[0].toObject()["description"].toString();
            qDebug() << "Weather:" << description;
        }
    }
}

void WeatherFetcher::onTimerTimeout()
{
    fetchWeather(m_currentCity);
}

#endif // WEATHERFETCHER_CPP
