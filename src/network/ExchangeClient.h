#ifndef EXCHANGECLIENT_H
#define EXCHANGECLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtWebSockets/QWebSocket>
#include <QPointer>
#include "../models/CandleData.h"

// Перечисления должны быть объявлены ПЕРЕД классом
enum class Interval {
    M1, M5, M15, M30, H1, H4, D1
};

enum class ClientState {
    Idle,
    LoadingHistory,
    Connected,
    Reconnecting,
    Error
};

class ExchangeClient : public QObject
{
    Q_OBJECT

public:
    explicit ExchangeClient(QObject *parent = nullptr);
    ~ExchangeClient();

    // Единый публичный метод, только загрузка истории (REST)
    void loadMarket(const QString& symbol, Interval interval, int limit = 100);

    // Явное управление реалтайм-подпиской (WebSocket), отдельно от загрузки и истории
    void startRealtime();
    void stopRealtime();

    // Статус
    bool isRealtimeConnected() const;
    ClientState state() const { return m_state; }
    QString symbol() const { return m_symbol; }
    Interval interval() const { return m_interval; }

signals:
    void candlesLoaded(const QList<CandleData>& candles);
    void newCandleTick(const CandleData& candle);
    void errorOccurred(const QString& error);
    void connectionStatusChanged(bool connected);

private slots:
    void onRestReplyFinished(QNetworkReply* reply);
    void onWebSocketConnected();
    void onWebSocketTextMessageReceived(const QString& message);
    void onWebSocketDisconnected();
    void onWebSocketError(QAbstractSocket::SocketError error);

private:
    // Вспомогательные методы
    void setState(ClientState newState);
    QList<CandleData> parseCandles(const QByteArray& data);
    void fetchHistory(const QString& symbol, Interval interval, int limit);
    void openRealtime(const QString& symbol, Interval interval);
    void closeRealtime();
    void scheduleReconnect();
    static QString intervalToString(Interval interval);

    // REST
    QNetworkAccessManager* m_networkManager;
    quint64 m_requestId = 0;

    // WebSocket
    QPointer<QWebSocket> m_webSocket;
    bool m_manualClose = false;

    // Состояние
    ClientState m_state = ClientState::Idle;
    QString m_symbol = "BTCUSDT";
    Interval m_interval = Interval::H1;
    qint64 m_lastCandleTime = 0;
};

#endif // EXCHANGECLIENT_H