#ifndef BYBITEXCHANGECLIENT_H
#define BYBITEXCHANGECLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtWebSockets/QWebSocket>
#include <QPointer>
#include <QTimer>
#include "IExchangeAdapter.h"
#include "../models/CandleData.h"

// Адаптер для Bybit (v5 API, спот-рынок).
// Отличия от Binance, которые здесь учтены:
// REST: /v5/market/kline с category=spot, интервалы в других обозначениях
// ("60" вместо "1h").
// REST отдаёт свечи от НОВЫХ к СТАРЫМ (обратный порядок относительно
// Binance) — переворачиваем при разборе.
// WebSocket не кодирует подписку в URL, а требует отдельного JSON-
// сообщения {"op":"subscribe","args":[...]} уже после подключения.
// Bybit разрывает "молчащее" соединение — раз в 20 секунд шлём ping.
// Быстрого тикера отдельных сделок здесь нет — верхняя панель цены
// остаётся источником Binance (см. MainController).
class BybitExchangeClient : public IExchangeAdapter
{
    Q_OBJECT

public:
    explicit BybitExchangeClient(QObject *parent = nullptr);
    ~BybitExchangeClient() override;

    QString exchangeName() const override { return "Bybit"; }

    void loadMarket(const QString& symbol, Interval interval, int limit = 100) override;
    void startRealtime() override;
    void stopRealtime() override;

    bool isRealtimeConnected() const override;
    QString symbol() const override { return m_symbol; }
    Interval interval() const override { return m_interval; }

private slots:
    void onRestReplyFinished(QNetworkReply* reply);
    void onWebSocketConnected();
    void onWebSocketTextMessageReceived(const QString& message);
    void onWebSocketDisconnected();
    void onWebSocketError(QAbstractSocket::SocketError error);
    void sendHeartbeat();

private:
    QList<CandleData> parseCandles(const QByteArray& data, Interval interval);
    void fetchHistory(const QString& symbol, Interval interval, int limit);
    void openRealtime(const QString& symbol, Interval interval);
    void closeRealtime();
    void scheduleReconnect();

    static QString intervalToBybit(Interval interval);
    static qint64 intervalDurationMs(Interval interval);

    QNetworkAccessManager* m_networkManager;
    quint64 m_requestId = 0;

    QPointer<QWebSocket> m_webSocket;
    QTimer* m_heartbeatTimer;
    bool m_manualClose = false;
    bool m_reconnectScheduled = false; // защита от двойного планирования переподключения

    QString m_symbol = "BTCUSDT";
    Interval m_interval = Interval::H1;
};

#endif // BYBITEXCHANGECLIENT_H