#include "ExchangeClient.h"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QDebug>
#include <QTimer>

ExchangeClient::ExchangeClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    qDebug() << "ExchangeClient created";
}

ExchangeClient::~ExchangeClient()
{
    closeRealtime();
    closeTradeStream();
    qDebug() << "ExchangeClient destroyed";
}


// Публичный метод - загружает только историю по REST
// Подключение WebSocket теперь управляет явно startRealtime()/stopRealtime()
void ExchangeClient::loadMarket(const QString& symbol, Interval interval, int limit)
{
    if (symbol.isEmpty()) {
        emit errorOccurred("Symbol is empty");
        return;
    }

    bool marketChanged = (symbol != m_symbol || interval != m_interval);

    if (marketChanged) {
        qDebug() << "Switching market to" << symbol << intervalToString(interval);


    // Старый WebSocket подписан на другой символ/интервал(он больше не актуален)
    // Закрываем старый WebSocket, но не открываем новый автоматически
    if (m_webSocket) {
        closeRealtime();
    }
    if (m_tradeWebSocket) {
        closeTradeStream();
    }

    // Обновляем параметры
    m_symbol = symbol;
    m_interval = interval;
  }

    // Загружаем историю (после её загрузки запустится WebSocket)
    setState(ClientState::LoadingHistory);
    fetchHistory(symbol, interval, limit);
}

    void ExchangeClient::startRealtime()
{
        if (m_symbol.isEmpty()) {
            emit errorOccurred("No market loaded yet");
            return;
        }

        if (isRealtimeConnected()) {
            qDebug() << "Realtime already connected";
            return;
        }

        qDebug() << "Starting realtime for" << m_symbol << intervalToString(m_interval);
        openRealtime(m_symbol, m_interval);
        openTradeStream(m_symbol);
}

void ExchangeClient::stopRealtime()
{
    if (!m_webSocket) {
        qDebug() <<"Realtime already stopped";
        return;
    }

    qDebug() << "Stopping realtime";
    closeRealtime();
    closeTradeStream();
    setState(ClientState::Idle);
  }


// REST (история)

void ExchangeClient::fetchHistory(const QString& symbol, Interval interval, int limit)
{
    QUrl url("https://api.binance.com/api/v3/klines");
    QUrlQuery query;
    query.addQueryItem("symbol", symbol);
    query.addQueryItem("interval", intervalToString(interval));
    query.addQueryItem("limit", QString::number(limit));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "DataMonitorPro/1.0");

    ++m_requestId;
    quint64 currentId = m_requestId;

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, currentId]() {
        if (currentId != m_requestId) {
            qDebug() << "Ignoring stale reply (id:" << currentId << ", current:" << m_requestId << ")";
            reply->deleteLater();
            return;
        }
        onRestReplyFinished(reply);
    });
}

void ExchangeClient::onRestReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "REST API error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        setState(ClientState::Error);
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QList<CandleData> candles = parseCandles(data);
    qDebug() << "REST API: loaded" << candles.size() << "candles";

    if (!candles.isEmpty()) {
        m_lastCandleTime = candles.last().closeTime;
        emit candlesLoaded(candles);
    } else {
        emit errorOccurred("Empty response from Binance");
        setState(ClientState::Error);
        return;
    }

    // WebSocket больше не открывается автоматически здесь
    // Для этого есть отдельный метод startRealtime()
    setState(isRealtimeConnected() ? ClientState::Connected : ClientState::Idle);
}


// WebSocket

void ExchangeClient::openRealtime(const QString& symbol, Interval interval)
{
    if (m_webSocket) {
        qDebug() << "WebSocket already exists, closing first";
        closeRealtime();
    }

    m_manualClose = false;  // сброс флага для авто-реконнекта

    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    qDebug() << "WebSocket" << static_cast<void*>(m_webSocket.data()) << "created";

    connect(m_webSocket, &QWebSocket::connected,
            this, &ExchangeClient::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &ExchangeClient::onWebSocketTextMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &ExchangeClient::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred,
            this, &ExchangeClient::onWebSocketError);

    QString wsUrl = QString("wss://stream.binance.com:9443/ws/%1@kline_%2")
                        .arg(symbol.toLower())
                        .arg(intervalToString(interval));

    qDebug() << "Connecting to Binance WebSocket:" << wsUrl;
    m_webSocket->open(QUrl(wsUrl));
}

void ExchangeClient::closeRealtime()
{
    if (!m_webSocket) {
        return;
    }

    qDebug() << "Closing WebSocket" << static_cast<void*>(m_webSocket.data());
    m_manualClose = true;  // отключаем авто-реконнект

    // Отключаем наши слоты
    disconnect(m_webSocket, nullptr, this, nullptr);

    // Сохраняем указатель и обнуляем
    QWebSocket* socket = m_webSocket;
    m_webSocket = nullptr;

    // Удаляем после disconnected
    connect(socket, &QWebSocket::disconnected,
            socket, &QObject::deleteLater,
            Qt::SingleShotConnection);

    socket->close();
    // Что бы QML узнало, что соединение закрыто и поменяло статус кнопки на Startrealtime
    emit connectionStatusChanged(false);
}

/* Отдельный, быстрый поток отдельных сделок (@trade) - только для верхней
 * панели цены. Обновляется на каждую сделку, а не раз в секунду как klines.
 * Без авто-рекконекта, если оборвется - просто ждет следующего startRealtime()
*/
void ExchangeClient::openTradeStream(const QString& symbol)
{
    if (m_tradeWebSocket) {
        qDebug() << "Trade WebSocket already exists, closing first";
        closeTradeStream();
    }

    m_tradeWebSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    qDebug() << "Trade WebSocket" << static_cast<void*>(m_tradeWebSocket.data()) << "created";

    connect(m_tradeWebSocket, &QWebSocket::textMessageReceived,
            this, &ExchangeClient::onTradeWebSocketTextMessageReceived);

    QString wsUrl = QString("wss://stream.binance.com:9443/ws/%1@trade")
                        .arg(symbol.toLower());

    qDebug() << "Connecting to Binance trade WebSocket:" << wsUrl;
    m_tradeWebSocket->open(QUrl(wsUrl));
}

void ExchangeClient::closeTradeStream()
{
    if (!m_tradeWebSocket) {
        return;
    }

    qDebug() << "Closing trade WebSocket" << static_cast<void*>(m_tradeWebSocket.data());

    disconnect(m_tradeWebSocket, nullptr, this, nullptr);

    QWebSocket* socket = m_tradeWebSocket;
    m_tradeWebSocket = nullptr;

    connect(socket, &QWebSocket::disconnected,
            socket, &QObject::deleteLater,
            Qt::SingleShotConnection);

    socket->close();
}


void ExchangeClient::onTradeWebSocketTextMessageReceived(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("p") || !root.contains("T")) {
        return;
    }

    double price = root["p"].toString().toDouble();
    qint64 tradeTime = root["T"].toVariant().toLongLong();

    emit tradeReceived(price, tradeTime);
}

bool ExchangeClient::isRealtimeConnected() const
{
    return m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState;
}

void ExchangeClient::scheduleReconnect()
{
    if (m_manualClose) return;
    if (m_state == ClientState::Error || m_state == ClientState::LoadingHistory) return;
    if (m_symbol.isEmpty()) return;

    QTimer::singleShot(3000, this, [this]() {
        if (!isRealtimeConnected() && m_state != ClientState::Error) {
            qDebug() << "Auto-reconnecting...";
            openRealtime(m_symbol, m_interval);
        }
    });
}


// WebSocket Слоты

void ExchangeClient::onWebSocketConnected()
{
    if (!m_webSocket) {
        qDebug() << "WebSocket destroyed before connection established";
        return;
    }

    qDebug() << "WebSocket" << static_cast<void*>(m_webSocket.data()) << "connected";
    emit connectionStatusChanged(true);
    setState(ClientState::Connected);
}

void ExchangeClient::onWebSocketDisconnected()
{
    qDebug() << "WebSocket disconnected";
    emit connectionStatusChanged(false);

    if (!m_manualClose) {
        setState(ClientState::Reconnecting);
        scheduleReconnect();
    } else {
        setState(ClientState::Idle);
    }
}

void ExchangeClient::onWebSocketError(QAbstractSocket::SocketError error)
{
    QString msg = QString("WebSocket error: %1").arg(error);
    qDebug() << msg;
    emit errorOccurred(msg);

    setState(ClientState::Error);
    if (!m_manualClose) {
        QTimer::singleShot(3000, this, [this]() {
            if (!isRealtimeConnected() && m_state == ClientState::Error) {
                qDebug() << "Reconnecting after error...";
                openRealtime(m_symbol, m_interval);
            }
        });
    }
}

void ExchangeClient::onWebSocketTextMessageReceived(const QString& message)
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Ignoring message - WebSocket not connected";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON message";
        return;
    }

    QJsonObject root = doc.object();

    // Ping от Binance
    if (root.contains("ping")) {
        QJsonObject pong;
        pong["pong"] = root["ping"].toVariant().toLongLong();
        m_webSocket->sendTextMessage(QJsonDocument(pong).toJson(QJsonDocument::Compact));
        return;
    }

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
    candle.isClosed = kline["x"].toBool();

    emit newCandleTick(candle);
}


// Вспомогательные методы

void ExchangeClient::setState(ClientState newState)
{
    if (m_state != newState) {
        m_state = newState;
        qDebug() << "State changed to:" << static_cast<int>(newState);
    }
}

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

    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isArray()) continue;
        QJsonArray arr = value.toArray();
        if (arr.size() < 7) continue;

        CandleData candle;
        candle.openTime = arr[0].toVariant().toLongLong();
        candle.open = arr[1].toString().toDouble();
        candle.high = arr[2].toString().toDouble();
        candle.low = arr[3].toString().toDouble();
        candle.close = arr[4].toString().toDouble();
        candle.volume = arr[5].toString().toDouble();
        candle.closeTime = arr[6].toVariant().toLongLong();
        candle.isClosed = true;
        candles.append(candle);
    }
    return candles;
}