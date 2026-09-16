#include "Bybittriangulararbitragemonitor.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QUrl>
#include <QDebug>

BybitTriangularArbitrageMonitor::BybitTriangularArbitrageMonitor(QObject *parent)
    : QObject(parent)
    , m_heartbeatTimer(new QTimer(this))
{
    m_heartbeatTimer->setInterval(20000); // Bybit требует ping не реже раза в 20с
    connect(m_heartbeatTimer, &QTimer::timeout, this, &BybitTriangularArbitrageMonitor::sendHeartbeat);
    qDebug() << "BybitTriangularArbitrageMonitor created";
}

BybitTriangularArbitrageMonitor::~BybitTriangularArbitrageMonitor()
{
    stop();
    qDebug() << "BybitTriangularArbitrageMonitor destroyed";
}

void BybitTriangularArbitrageMonitor::start()
{
    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Bybit triangular arbitrage monitor already running";
        return;
    }

    // Сбрасываем старые цены — иначе после Stop/Start недолго будет висеть
    // расчёт по устаревшим данным до прихода первых свежих цен.
    m_bidA = m_askA = m_bidB = m_askB = m_bidC = m_askC = 0.0;

    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(m_webSocket, &QWebSocket::connected,
            this, &BybitTriangularArbitrageMonitor::onConnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &BybitTriangularArbitrageMonitor::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &BybitTriangularArbitrageMonitor::onDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred,
            this, &BybitTriangularArbitrageMonitor::onError);

    qDebug() << "Connecting Bybit triangular arbitrage stream";
    m_webSocket->open(QUrl("wss://stream.bybit.com/v5/public/spot"));
}

void BybitTriangularArbitrageMonitor::stop()
{
    if (!m_webSocket) {
        return;
    }

    qDebug() << "Stopping Bybit triangular arbitrage monitor";
    m_heartbeatTimer->stop();

    disconnect(m_webSocket, nullptr, this, nullptr);

    QWebSocket* socket = m_webSocket;
    m_webSocket = nullptr;

    connect(socket, &QWebSocket::disconnected,
            socket, &QObject::deleteLater,
            Qt::SingleShotConnection);
    socket->close();

    m_isRunning = false;
    emit runningChanged();
}

void BybitTriangularArbitrageMonitor::onConnected()
{
    if (!m_webSocket) return;

    qDebug() << "Bybit triangular arbitrage stream connected, subscribing...";

    // Подписка отправляется отдельным сообщением ПОСЛЕ подключения —
    // как и у BybitExchangeClient. Все три пары — в одной подписке, Bybit
    // поддерживает несколько топиков за раз.
    QJsonObject sub;
    sub["op"] = "subscribe";
    QJsonArray args;
    args.append(QString("orderbook.1.%1").arg(m_symbolA));
    args.append(QString("orderbook.1.%1").arg(m_symbolB));
    args.append(QString("orderbook.1.%1").arg(m_symbolC));
    sub["args"] = args;

    m_webSocket->sendTextMessage(QJsonDocument(sub).toJson(QJsonDocument::Compact));

    m_heartbeatTimer->start();
    m_isRunning = true;
    emit runningChanged();
}

void BybitTriangularArbitrageMonitor::onDisconnected()
{
    qDebug() << "Bybit triangular arbitrage stream disconnected";
    m_heartbeatTimer->stop();

    // Обнуляем указатель сразу — иначе start() увидит "живой" указатель на
    // мёртвый сокет и молча откажется переподключаться (та самая ошибка,
    // что уже была найдена и исправлена в Binance-версии этого класса).
    if (m_webSocket) {
        QWebSocket* socket = m_webSocket;
        m_webSocket = nullptr;
        disconnect(socket, nullptr, this, nullptr);
        socket->deleteLater();
    }

    m_isRunning = false;
    emit runningChanged();
}

void BybitTriangularArbitrageMonitor::onError(QAbstractSocket::SocketError error)
{
    QString msg = QString("Bybit triangular arbitrage stream error: %1").arg(static_cast<int>(error));
    qDebug() << msg;
    emit errorOccurred(msg);
}

void BybitTriangularArbitrageMonitor::sendHeartbeat()
{
    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject ping;
        ping["op"] = "ping";
        m_webSocket->sendTextMessage(QJsonDocument(ping).toJson(QJsonDocument::Compact));
    }
}

void BybitTriangularArbitrageMonitor::onTextMessageReceived(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return;
    }

    QJsonObject root = doc.object();

    // Ответ на подписку/пинг — не содержит "topic", просто игнорируем.
    if (!root.contains("topic")) return;

    QString topic = root["topic"].toString();
    if (!topic.startsWith("orderbook.1.")) return;

    QJsonObject data = root["data"].toObject();
    QString symbol = data.value("s").toString();

    QJsonArray bids = data.value("b").toArray();
    QJsonArray asks = data.value("a").toArray();
    if (bids.isEmpty() || asks.isEmpty()) return;

    double bid = bids.first().toArray().at(0).toString().toDouble();
    double ask = asks.first().toArray().at(0).toString().toDouble();
    if (bid <= 0.0 || ask <= 0.0) return;

    bool changed = false;

    if (symbol == m_symbolA) {
        m_bidA = bid; m_askA = ask; changed = true;
    } else if (symbol == m_symbolB) {
        m_bidB = bid; m_askB = ask; changed = true;
    } else if (symbol == m_symbolC) {
        m_bidC = bid; m_askC = ask; changed = true;
    }

    if (changed) {
        emit pricesChanged();
        recomputeProfit();
    }
}

void BybitTriangularArbitrageMonitor::recomputeProfit()
{
    // Ждём, пока придут цены по всем трём парам хотя бы раз.
    if (m_askA <= 0.0 || m_askB <= 0.0 || m_askC <= 0.0 ||
        m_bidA <= 0.0 || m_bidB <= 0.0 || m_bidC <= 0.0) {
        return;
    }

    double feeFactor = 1.0 - (m_feePercent / 100.0);

    // Прямой цикл: 1 USDT -> BTC (по ask BTCUSDT) -> ETH (по ask ETHBTC)
    // -> обратно в USDT (по bid ETHUSDT).
    double amountBTC = (1.0 / m_askA) * feeFactor;
    double amountETH_fwd = (amountBTC / m_askC) * feeFactor;
    double finalUSDT_fwd = (amountETH_fwd * m_bidB) * feeFactor;
    m_forwardProfitPercent = (finalUSDT_fwd - 1.0) * 100.0;

    // Обратный цикл: 1 USDT -> ETH (по ask ETHUSDT) -> BTC (по bid ETHBTC)
    // -> обратно в USDT (по bid BTCUSDT).
    double amountETH_rev = (1.0 / m_askB) * feeFactor;
    double amountBTC_rev = (amountETH_rev * m_bidC) * feeFactor;
    double finalUSDT_rev = (amountBTC_rev * m_bidA) * feeFactor;
    m_reverseProfitPercent = (finalUSDT_rev - 1.0) * 100.0;

    m_lastUpdateTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    emit profitChanged();
}

void BybitTriangularArbitrageMonitor::setFeePercent(double value)
{
    if (qFuzzyCompare(m_feePercent, value)) {
        return;
    }
    m_feePercent = value;
    emit feePercentChanged();
    recomputeProfit();
}