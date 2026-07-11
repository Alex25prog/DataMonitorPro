#include "ExchangeClient.h"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

ExchangeClient::ExchangeClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_realtimeTimer(new QTimer(this))
    , m_symbol("BTCUSDT")
    , m_interval(Interval::H1)

{
    // Подключаем обработчик ответов
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &ExchangeClient::onReplyFinished);

    connect(m_realtimeTimer, &QTimer::timeout,
            this, &ExchangeClient::onRealtimeUpdate);
}

ExchangeClient::~ExchangeClient()
{
    stopRealtimeUpdates();
}

QString ExchangeClient::intervalToString(Interval interval) const
{
    switch (interval) {
    case Interval::M1: return "1m";
    case Interval::M5: return "5m";
    case Interval::M15: return "15m";
    case Interval::M30: return "30m";
    case Interval::H1: return "1h";
    case Interval::H4: return "4h";
    case Interval::D1: return "1d";
    default:           return "1h";
    }
}

void ExchangeClient::loadHistory(const QString& symbol, Interval interval, int limit)
{
    m_symbol = symbol;
    m_interval = interval;
    fetchCandles(symbol, interval, limit);
}

void ExchangeClient::fetchCandles(const QString& symbol, Interval interval, int limit)
{
    QUrl url("https://api.binance.com/api/v3/klines");
    QUrlQuery query;
    query.addQueryItem("symbol", symbol);
    query.addQueryItem("interval", intervalToString(interval));
    query.addQueryItem("limit", QString::number(limit));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "DataMonitorPro/1.0");

    qDebug() << "Fetching candles for" << symbol << "interval:" << intervalToString(interval);
    m_manager->get(request);
}

void ExchangeClient::startRealtimeUpdates(const QString& symbol, Interval interval)
{
    if (m_isRealtimeRunning) {
        qDebug() << "Realtime updates already running";
        return;
    }

    m_symbol = symbol;
    m_interval = interval;

    // Загружаем историю для контекста
    fetchCandles(symbol, interval, 100);

    // Запускаем таймер для обновления каждые 30 секкунд
    m_realtimeTimer->start(30000);
    m_isRealtimeRunning = true;

    qDebug() << "Realtime updates started for" << symbol;
}

void ExchangeClient::stopRealtimeUpdates()
{
    if (m_isRealtimeRunning) {
        m_realtimeTimer->stop();
        m_isRealtimeRunning = false;
        qDebug() << "Realtime updates stopped";
    }
}

void ExchangeClient::onReplyFinished(QNetworkReply* reply)
{

    // Отладочные выводы
    qDebug() << "===Reply received===";
    qDebug() << "URL:" << reply->url().toString();
    qDebug() << "Error code:" << reply->error();
    qDebug() << "Error string:" << reply->errorString();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Exchange API error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QList<CandleData> candles = parseCandles(data);

    // Проверяем, не пришла ли ошибка от Binance
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("code") && obj.contains("msg")) {
            qDebug() << "Binance API error:" << obj["msg"].isString();
            emit errorOccurred("Binance error: " + obj["msg"].toString());
            reply->deleteLater();
            return;
        }
    }

    QList<CandleData> parsedCandles = parseCandles(data);
    qDebug() << "Parsed" << parsedCandles.size() << "candles";

    if (!parsedCandles.isEmpty()) {
        emit candlesLoaded(parsedCandles);
        qDebug() << "candlesLoaded emitted with" << parsedCandles.size() << "candles";

        // Запоминаем время последней свечи для обновлений
        m_lastCandleTime = parsedCandles.last().closeTime;
    } else {
        qDebug() << "WARNING: No candles parsed from response!";
        qDebug() << "Raw data (first 200 chars):" << data.left(200);
    }

    reply->deleteLater();
}

void ExchangeClient::onRealtimeUpdate()
{
    if (m_symbol.isEmpty()) {
        qDebug() << "No symbol selected for realtime update";
        return;
    }

    // Запрашиваем только последнюю свечу
    QUrl url("https://api.binance.com/api/v3/klines");
    QUrlQuery query;
    query.addQueryItem("symbol", m_symbol);
    query.addQueryItem("interval", intervalToString(m_interval));
    query.addQueryItem("limit", "2");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "DataMonitorPro/1.0");

    m_manager->get(request);
}

QList<CandleData> ExchangeClient::parseCandles(const QByteArray& data)
{
    QList<CandleData> candles;
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isArray()) {
        qDebug() << "Invalid JSON from Binance";
        return candles;
    }

    QJsonArray array = doc.array();

    for (const QJsonValue& value : array) {
        if (!value.isArray()) continue;

        QJsonArray candleArray = value.toArray();

        //Binance возвращает 12 полей, проверяем хотя бы 7
        if (candleArray.size() < 7) {
            qDebug() << "Candle array too small:" << candleArray.size();
            continue;
        }

        CandleData candle;
        candle.openTime = candleArray[0].toVariant().toLongLong();
        candle.open = candleArray[1].toString().toDouble();
        candle.high = candleArray[2].toString().toDouble();
        candle.low = candleArray[3].toString().toDouble();
        candle.close = candleArray[4].toString().toDouble();
        candle.volume = candleArray[5].toString().toDouble();
        candle.closeTime = candleArray[6].toVariant().toLongLong();
        candle.isClosed = true;

        candles.append(candle);
    }

    qDebug() << "Parsed" << candles.size() << "candles";
    return candles;
}