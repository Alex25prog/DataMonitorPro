#include "BybitExchangeClient.h"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QDebug>
#include <algorithm>

BybitExchangeClient::BybitExchangeClient(QObject *parent)
    : IExchangeAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_heartbeatTimer(new QTimer(this))
{
    m_heartbeatTimer->setInterval(20000); // Bybit требует ping не реже раза в 20с
    connect(m_heartbeatTimer, &QTimer::timeout, this, &BybitExchangeClient::sendHeartbeat);
    qDebug() << "BybitExchangeClient created";
}

BybitExchangeClient::~BybitExchangeClient()
{
    closeRealtime();
    qDebug() << "BybitExchangeClient destroyed";
}

void BybitExchangeClient::loadMarket(const QString& symbol, Interval interval, int limit)
{
    if (symbol.isEmpty()) {
        emit errorOccurred("Symbol is empty");
        return;
    }

    bool marketChanged = (symbol != m_symbol || interval != m_interval);
    if (marketChanged) {
        qDebug() << "Bybit: switching market to" << symbol << intervalToBybit(interval);
        if (m_webSocket) {
            closeRealtime();
        }
        m_symbol = symbol;
        m_interval = interval;
    }

    fetchHistory(symbol, interval, limit);
}

void BybitExchangeClient::fetchHistory(const QString& symbol, Interval interval, int limit)
{
    QUrl url("https://api.bybit.com/v5/market/kline");
    QUrlQuery query;
    query.addQueryItem("category", "spot");
    query.addQueryItem("symbol", symbol);
    query.addQueryItem("interval", intervalToBybit(interval));
    query.addQueryItem("limit", QString::number(limit));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "DataMonitorPro/1.0");

    ++m_requestId;
    quint64 currentId = m_requestId;

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, currentId]() {
        if (currentId != m_requestId) {
            reply->deleteLater();
            return;
        }
        onRestReplyFinished(reply);
    });
}

void BybitExchangeClient::onRestReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Bybit REST API error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QList<CandleData> candles = parseCandles(data, m_interval);
    qDebug() << "Bybit REST API: loaded" << candles.size() << "candles";

    if (!candles.isEmpty()) {
        emit candlesLoaded(candles);
    } else {
        emit errorOccurred("Empty response from Bybit");
    }
}

QList<CandleData> BybitExchangeClient::parseCandles(const QByteArray& data, Interval interval)
{
    QList<CandleData> candles;
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Bybit: invalid JSON";
        return candles;
    }

    QJsonObject root = doc.object();
    if (root.value("retCode").toInt(-1) != 0) {
        qDebug() << "Bybit API error:" << root.value("retMsg").toString();
        return candles;
    }

    QJsonArray list = root.value("result").toObject().value("list").toArray();
    qint64 durationMs = intervalDurationMs(interval);

    for (const QJsonValue& value : list) {
        if (!value.isArray()) continue;
        QJsonArray arr = value.toArray();
        if (arr.size() < 6) continue;

        CandleData candle;
        candle.openTime = arr[0].toVariant().toLongLong();
        candle.open = arr[1].toString().toDouble();
        candle.high = arr[2].toString().toDouble();
        candle.low = arr[3].toString().toDouble();
        candle.close = arr[4].toString().toDouble();
        candle.volume = arr[5].toString().toDouble();
        candle.closeTime = candle.openTime + durationMs;
        candle.isClosed = true;
        candles.append(candle);
    }

    // Bybit отдаёт от новых к старым — переворачиваем, чтобы совпадало
    // с ожидаемым во всём приложении порядком (от старых к новым).
    std::reverse(candles.begin(), candles.end());
    return candles;
}

void BybitExchangeClient::startRealtime()
{
    if (m_symbol.isEmpty()) {
        emit errorOccurred("No market loaded yet");
        return;
    }
    if (isRealtimeConnected()) {
        qDebug() << "Bybit realtime already connected";
        return;
    }
    openRealtime(m_symbol, m_interval);
}

void BybitExchangeClient::stopRealtime()
{
    if (!m_webSocket) return;
    closeRealtime();
}

void BybitExchangeClient::openRealtime(const QString& symbol, Interval interval)
{
    if (m_webSocket) {
        closeRealtime();
    }

    m_manualClose = false;
    m_reconnectScheduled = false;

    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(m_webSocket, &QWebSocket::connected,
            this, &BybitExchangeClient::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &BybitExchangeClient::onWebSocketTextMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &BybitExchangeClient::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred,
            this, &BybitExchangeClient::onWebSocketError);

    qDebug() << "Connecting to Bybit WebSocket for" << symbol << intervalToBybit(interval);
    m_webSocket->open(QUrl("wss://stream.bybit.com/v5/public/spot"));
}

void BybitExchangeClient::closeRealtime()
{
    if (!m_webSocket) return;

    m_manualClose = true;
    m_reconnectScheduled = false;
    m_heartbeatTimer->stop();

    disconnect(m_webSocket, nullptr, this, nullptr);

    QWebSocket* socket = m_webSocket;
    m_webSocket = nullptr;

    connect(socket, &QWebSocket::disconnected,
            socket, &QObject::deleteLater,
            Qt::SingleShotConnection);
    socket->close();

    emit connectionStatusChanged(false);
}

void BybitExchangeClient::onWebSocketConnected()
{
    if (!m_webSocket) return;

    qDebug() << "Bybit WebSocket connected, subscribing...";

    // Подписка отправляется отдельным сообщением ПОСЛЕ подключения —
    // в отличие от Binance, где поток кодируется прямо в URL.
    // Bybit поддерживает несколько топиков в одной подписке — в отличие от
    // Binance, отдельный сокет под быстрый поток цены не нужен: свечи и
    // отдельные сделки (publicTrade) идут через ОДНО и то же соединение.
    QJsonObject sub;
    sub["op"] = "subscribe";
    QJsonArray args;
    args.append(QString("kline.%1.%2").arg(intervalToBybit(m_interval), m_symbol));
    args.append(QString("publicTrade.%1").arg(m_symbol));
    sub["args"] = args;

    m_webSocket->sendTextMessage(QJsonDocument(sub).toJson(QJsonDocument::Compact));

    m_heartbeatTimer->start();
    emit connectionStatusChanged(true);
}

void BybitExchangeClient::onWebSocketDisconnected()
{
    qDebug() << "Bybit WebSocket disconnected";
    m_heartbeatTimer->stop();
    emit connectionStatusChanged(false);

    if (!m_manualClose) {
        scheduleReconnect();
    }
}

void BybitExchangeClient::onWebSocketError(QAbstractSocket::SocketError error)
{
    QString msg = QString("Bybit WebSocket error: %1").arg(static_cast<int>(error));
    qDebug() << msg;
    emit errorOccurred(msg);

    if (!m_manualClose) {
        scheduleReconnect();
    }
}

void BybitExchangeClient::scheduleReconnect()
{
    if (m_manualClose || m_symbol.isEmpty()) return;

    // Защита от двойного планирования: при реальном сбое соединения Qt
    // почти всегда шлёт errorOccurred И disconnected для ОДНОГО И ТОГО ЖЕ
    // события — без этого флага получалось два независимых таймера и два
    // новых сокета поверх друг друга (см. "QIODevice::write: device not
    // open" в реальном логе).
    if (m_reconnectScheduled) return;
    m_reconnectScheduled = true;

    QTimer::singleShot(3000, this, [this]() {
        m_reconnectScheduled = false;
        if (!isRealtimeConnected()) {
            qDebug() << "Bybit: auto-reconnecting...";
            openRealtime(m_symbol, m_interval);
        }
    });
}

void BybitExchangeClient::sendHeartbeat()
{
    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject ping;
        ping["op"] = "ping";
        m_webSocket->sendTextMessage(QJsonDocument(ping).toJson(QJsonDocument::Compact));
    }
}

void BybitExchangeClient::onWebSocketTextMessageReceived(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject root = doc.object();

    // Ответ на подписку/пинг — не содержит "topic", просто игнорируем.
    if (!root.contains("topic")) return;

    QString topic = root["topic"].toString();

    if (topic.startsWith("kline.")) {
        QJsonArray dataArr = root["data"].toArray();
        if (dataArr.isEmpty()) return;

        QJsonObject k = dataArr.first().toObject();

        CandleData candle;
        candle.openTime = k["start"].toVariant().toLongLong();
        candle.closeTime = k["end"].toVariant().toLongLong();
        candle.open = k["open"].toString().toDouble();
        candle.high = k["high"].toString().toDouble();
        candle.low = k["low"].toString().toDouble();
        candle.close = k["close"].toString().toDouble();
        candle.volume = k["volume"].toString().toDouble();
        candle.isClosed = k["confirm"].toBool();

        emit newCandleTick(candle);
        return;
    }

    if (topic.startsWith("publicTrade.")) {
        // Bybit может прислать НЕСКОЛЬКО сделок одним сообщением (пачкой,
        // если они произошли почти одновременно) — эмитим на каждую
        // отдельно, чтобы сохранить ту же гранулярность "тик за тиком",
        // что и у Binance @trade.
        QJsonArray dataArr = root["data"].toArray();
        for (const QJsonValue& v : dataArr) {
            QJsonObject trade = v.toObject();
            double price = trade["p"].toString().toDouble();
            qint64 tradeTime = trade["T"].toVariant().toLongLong();
            if (price > 0.0) {
                emit tradeReceived(price, tradeTime);
            }
        }
        return;
    }
}

bool BybitExchangeClient::isRealtimeConnected() const
{
    return m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState;
}

QString BybitExchangeClient::intervalToBybit(Interval interval)
{
    switch (interval) {
    case Interval::M1:  return "1";
    case Interval::M5:  return "5";
    case Interval::M15: return "15";
    case Interval::M30: return "30";
    case Interval::H1:  return "60";
    case Interval::H4:  return "240";
    case Interval::D1:  return "D";
    default:             return "60";
    }
}

qint64 BybitExchangeClient::intervalDurationMs(Interval interval)
{
    switch (interval) {
    case Interval::M1:  return 60LL * 1000;
    case Interval::M5:  return 5LL * 60 * 1000;
    case Interval::M15: return 15LL * 60 * 1000;
    case Interval::M30: return 30LL * 60 * 1000;
    case Interval::H1:  return 60LL * 60 * 1000;
    case Interval::H4:  return 4LL * 60 * 60 * 1000;
    case Interval::D1:  return 24LL * 60 * 60 * 1000;
    default:             return 60LL * 60 * 1000;
    }
}