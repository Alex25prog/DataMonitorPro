#ifndef BINANCEEXCHANGECLIENT_H
#define BINANCEEXCHANGECLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtWebSockets/QWebSocket>
#include <QPointer>
#include "IExchangeAdapter.h"
#include "../models/CandleData.h"

// ClientState — внутреннее состояние конкретно этой реализации (Binance),
// не часть общего контракта IExchangeAdapter.
enum class ClientState {
    Idle,
    LoadingHistory,
    Connected,
    Reconnecting,
    Error
};

class BinanceExchangeClient : public IExchangeAdapter
{
    Q_OBJECT

public:
    explicit BinanceExchangeClient(QObject *parent = nullptr);
    ~BinanceExchangeClient() override;

    QString exchangeName() const override { return "Binance"; }

    // Единый публичный метод, только загрузка истории (REST)
    void loadMarket(const QString& symbol, Interval interval, int limit = 100) override;

    // Явное управление реалтайм-подпиской (WebSocket), отдельно от загрузки и истории
    void startRealtime() override;
    void stopRealtime() override;

    // Статус
    bool isRealtimeConnected() const override;
    ClientState state() const { return m_state; }
    QString symbol() const override { return m_symbol; }
    Interval interval() const override { return m_interval; }

//signals:
    /**Отдельный, гораздо более быстрый поток цены (@trade) - обновляется на
       каждую реальную сделку на бирже, а не раз в секунду как поток свечей
       используется только для верхней панели цены, график свечей не трогает
       ВАЖНО: этот сигнал специфичен для Binance, не часть IExchangeAdapter —
       у других бирж пока нет аналогичного быстрого тикера.
    **/
    //void tradeReceived(double price, qint64 tradeTimeMs);

private slots:
    void onRestReplyFinished(QNetworkReply* reply);
    void onWebSocketConnected();
    void onWebSocketTextMessageReceived(const QString& message);
    void onWebSocketDisconnected();
    void onWebSocketError(QAbstractSocket::SocketError error);
    void onTradeWebSocketTextMessageReceived(const QString& message);
    void onTradeWebSocketDisconnected();
    void onTradeWebSocketError(QAbstractSocket::SocketError error);

private:
    // Вспомогательные методы
    void setState(ClientState newState);
    QList<CandleData> parseCandles(const QByteArray& data);
    void fetchHistory(const QString& symbol, Interval interval, int limit);
    void openRealtime(const QString& symbol, Interval interval);
    void closeRealtime();
    void openTradeStream(const QString& symbol);
    void closeTradeStream();
    void scheduleTradeReconnect();
    void scheduleReconnect();
    static QString intervalToString(Interval interval);

    // REST
    QNetworkAccessManager* m_networkManager;
    quint64 m_requestId = 0;

    // WebSocket (свечи)
    QPointer<QWebSocket> m_webSocket;
    bool m_manualClose = false;
    bool m_reconnectScheduled = false; // Защита от двойного планирования переподключения

    // WebSocket (быстрый поток отдельных сделок, только для цены)
    QPointer<QWebSocket> m_tradeWebSocket;
    bool m_tradeManualClose = false;
    bool m_tradeReconnectScheduled = false;

    // Состояние
    ClientState m_state = ClientState::Idle;
    QString m_symbol = "BTCUSDT";
    Interval m_interval = Interval::H1;
    qint64 m_lastCandleTime = 0;
};

#endif // BINANCEEXCHANGECLIENT_H