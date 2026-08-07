#include "TriangularArbitrageMonitor.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QUrl>
#include <QDebug>

TriangularArbitrageMonitor::TriangularArbitrageMonitor(QObject *parent)
    : QObject(parent)
{
    qDebug() << "TriangularArbitrageMonitor created";
}

TriangularArbitrageMonitor::~TriangularArbitrageMonitor()
{
    stop();
    qDebug() << "TriangularArbitrageMonitor destroyed";
}

void TriangularArbitrageMonitor::start()
{
    if (m_webSocket) {
        qDebug() << "Triangular arbitrage monitor already running";
        return;
    }

    // Сбрасываем старые цены (иначе после Stop/Start недолго будет висеть
    // Расчет по устаревшим данным до прихода первых свежих bookTicker
    m_bidA = m_askA = m_bidB = m_askB = m_bidC = m_askC = 0.0;

    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(m_webSocket, &QWebSocket::connected,
            this, &TriangularArbitrageMonitor::onConnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &TriangularArbitrageMonitor::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &TriangularArbitrageMonitor::onDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred,
            this, &TriangularArbitrageMonitor::onError);

    // Комбинированный поток — одно соединение вместо трёх отдельных.
    // Формат сообщений: {"stream":"btcusdt@bookTicker","data":{...}}
    QString streams = QString("%1@bookTicker/%2@bookTicker/%3@bookTicker")
                          .arg(m_symbolA.toLower())
                          .arg(m_symbolB.toLower())
                          .arg(m_symbolC.toLower());

    QString url = QString("wss://stream.binance.com:9443/stream?streams=%1").arg(streams);

    qDebug() << "Connecting triangular arbitrage stream:" << url;
    m_webSocket->open(QUrl(url));
}

void TriangularArbitrageMonitor::stop()
{
    if (!m_webSocket) {
        return;
    }

    qDebug() << "Stopping triangular arbitrage monitor";

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

void TriangularArbitrageMonitor::onConnected()
{
    qDebug() << "Triangular arbitrage stream connected";
    m_isRunning = true;
    emit runningChanged();
}

void TriangularArbitrageMonitor::onDisconnected()
{
    qDebug() << "Triangular arbitrage stream disconnected";
    m_isRunning = false;
    emit runningChanged();
}

void TriangularArbitrageMonitor::onError(QAbstractSocket::SocketError error)
{
    QString msg = QString("Triangular arbitrage stream error: %1").arg(static_cast<int>(error));
    qDebug() << msg;
    emit errorOccurred(msg);
}

void TriangularArbitrageMonitor::onTextMessageReceived(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("stream") || !root.contains("data")) {
        return;
    }

    QString stream = root["stream"].toString().toLower();
    QJsonObject data = root["data"].toObject();

    double bid = data.value("b").toString().toDouble();
    double ask = data.value("a").toString().toDouble();
    if (bid <= 0.0 || ask <= 0.0) {
        return;
    }

    bool changed = false;

    if (stream.startsWith(m_symbolA.toLower())) {
        m_bidA = bid; m_askA = ask; changed = true;
    } else if (stream.startsWith(m_symbolB.toLower())) {
        m_bidB = bid; m_askB = ask; changed = true;
    } else if (stream.startsWith(m_symbolC.toLower())) {
        m_bidC = bid; m_askC = ask; changed = true;
    }

    if (changed) {
        emit pricesChanged();
        recomputeProfit();
    }
}

void TriangularArbitrageMonitor::recomputeProfit()
{
    // Ждём, пока придут цены по всем трём парам хотя бы раз.
    if (m_askA <= 0.0 || m_askB <= 0.0 || m_askC <= 0.0 ||
        m_bidA <= 0.0 || m_bidB <= 0.0 || m_bidC <= 0.0) {
        return;
    }

    double feeFactor = 1.0 - (m_feePercent / 100.0);

    // Прямой цикл: 1 USDT -> BTC (по ask BTCUSDT) -> ETH (по ask ETHBTC)
    // -> обратно в USDT (по bid ETHUSDT). ETHBTC/ETHUSDT/BTCUSDT — везде
    // базовый актив первый, котируемый — второй (стандарт Binance).
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

void TriangularArbitrageMonitor::setFeePercent(double value)
{
    if (qFuzzyCompare(m_feePercent, value)) {
        return;
    }
    m_feePercent = value;
    emit feePercentChanged();
    recomputeProfit();
}