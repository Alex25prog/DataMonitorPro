#include "ExchangeClient.h"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QDebug>

// ============================================================
// Конструктор / Деструктор
// ============================================================
ExchangeClient::ExchangeClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_symbol("BTCUSDT")
    , m_interval(Interval::H1)
{
    // Подключаем сигналы WebSocket
    connect(m_webSocket, &QWebSocket::connected,
            this, &ExchangeClient::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &ExchangeClient::onWebSocketTextMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &ExchangeClient::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred,
            this, &ExchangeClient::onWebSocketError);

    qDebug() << "ExchangeClient created";
}

ExchangeClient::~ExchangeClient()
{
    stopRealtimeUpdates();
    qDebug() << "ExchangeClient destroyed";
}

// ============================================================
// Вспомогательные методы
// ============================================================
QString ExchangeClient::intervalToString(Interval interval)
{
    switch (interval) {
    case Interval::M1:  return "1m";
    case Interval::M5:  return "5m";
    case Interval::M15: return "15m";
    case Interval::M30: return "30m";
    case Interval::H1:  return "1h";
    case Interval::H4:  return "4h";
    case Interval::D1:  return "1d";
    default:            return "1h";
    }
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
        if (candleArray.size() < 7) continue;

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

    return candles;
}

// ============================================================
// REST API — загрузка истории
// ============================================================
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

    qDebug() << "Fetching candles for" << symbol << "interval:" << intervalToString(interval) << "limit:" << limit;

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        onRestReplyFinished(reply);
    });
}

void ExchangeClient::loadHistory(const QString& symbol, Interval interval, int limit)
{
    m_symbol = symbol;
    m_interval = interval;
    fetchCandles(symbol, interval, limit);
}

void ExchangeClient::onRestReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "REST API error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QList<CandleData> candles = parseCandles(data);
    qDebug() << "REST API: loaded" << candles.size() << "candles";

    if (!candles.isEmpty()) {
        emit candlesLoaded(candles);
        m_lastCandleTime = candles.last().closeTime;
    }
}

// ============================================================
// WebSocket — реальное время
// ============================================================
void ExchangeClient::startRealtimeUpdates(const QString& symbol, Interval interval)
{
    // Если уже подключены к этому же символу, не переключаемся
    if (m_isRealtimeConnected && m_symbol.toLower() == symbol.toLower()) {
        qDebug() << "WebSocket already connected to" << symbol;
        return;
    }

    // Если подключены к другому символу-> закрываем
    if (m_isRealtimeConnected) {
        stopRealtimeUpdates();
    }

    m_symbol = symbol;
    m_interval = interval;

    // Binance WebSocket URL для свечей (символ в нижнем регистре)
    QString wsUrl = QString("wss://stream.binance.com:9443/ws/%1@kline_%2")
                        .arg(symbol.toLower())
                        .arg(intervalToString(interval));

    qDebug() << "Connecting to Binance WebSocket:" << wsUrl;
    m_webSocket->open(QUrl(wsUrl));
}

void ExchangeClient::stopRealtimeUpdates()
{
    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->close();
        qDebug() << "WebSocket closed";
    }
    m_isRealtimeConnected = false;
}

bool ExchangeClient::isRealtimeConnected() const
{
    return m_isRealtimeConnected;
}

void ExchangeClient::onWebSocketConnected()
{
    m_isRealtimeConnected = true;
    qDebug() << "WebSocket connected to Binance!";
    emit connectionStatusChanged(true);
}

void ExchangeClient::onWebSocketDisconnected()
{
    m_isRealtimeConnected = false;
    qDebug() << "WebSocket disconnected from Binance";
    emit connectionStatusChanged(false);
}

void ExchangeClient::onWebSocketError(QAbstractSocket::SocketError error)
{
    QString errorMsg = QString("WebSocket error: %1").arg(error);
    qDebug() << errorMsg;
    emit errorOccurred(errorMsg);
    m_isRealtimeConnected = false;
}

// ============================================================
// Парсинг WebSocket сообщений
// ============================================================
void ExchangeClient::onWebSocketTextMessageReceived(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid WebSocket message";
        return;
    }

    QJsonObject root = doc.object();

    // Проверяем, что это сообщение о свече
    if (!root.contains("k")) {
        return;
    }

    QJsonObject kline = root["k"].toObject();

    CandleData candle;
    candle.openTime = kline["t"].toVariant().toLongLong();
    candle.closeTime = kline["T"].toVariant().toLongLong();
    candle.open = kline["o"].toString().toDouble();
    candle.high = kline["h"].toString().toDouble();
    candle.low = kline["l"].toString().toDouble();
    candle.close = kline["c"].toString().toDouble();
    candle.volume = kline["v"].toString().toDouble();
    candle.isClosed = kline["x"].toBool();  // true = свеча закрылась

    // Отправляем сигнал о новом тике
    emit newCandleTick(candle);
}