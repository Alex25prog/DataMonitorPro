#ifndef EXCHANGECLIENT_H
#define EXCHANGECLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtWebSockets/QWebSocket>
//#include <QTimer>
#include "../models/CandleData.h"

// Интервалы свечей
enum class Interval {
    M1,  // 1 минута
    M5,  // 5 минут
    M15, // 15 минут
    M30, // 30 минут
    H1,  // 1 час
    H4,  // 4 часа
    D1   // 1 день
};

class ExchangeClient : public QObject
{
    Q_OBJECT

public:
    explicit ExchangeClient(QObject *parent = nullptr);
    ~ExchangeClient();

    // Остновные методы
    void loadHistory(const QString& symbol, Interval interval, int limit = 100); // REST API загрузка истории

    // WebSocket реальное время
    void startRealtimeUpdates(const QString& symbol, Interval interval);
    void stopRealtimeUpdates();
    bool isRealtimeConnected() const;

    // Утилиты
    void setSymbol(const QString& symbol) { m_symbol = symbol; }
    QString symbol() const { return m_symbol;}
    static QString intervalToString(Interval interval);
    Interval interval() const { return m_interval; }


signals:
    void candlesLoaded(const QList<CandleData>& candles);
    void newCandleTick(const CandleData& candle);
    void errorOccurred(const QString& error);
    void connectionStatusChanged(bool connected);

private slots:
    //  REST API
    void onRestReplyFinished(QNetworkReply* reply);
    //void onRealtimeUpdate();


    // WebSocket
    void onWebSocketConnected();
    void onWebSocketTextMessageReceived(const QString& message);
    void onWebSocketDisconnected();
    void onWebSocketError(QAbstractSocket::SocketError error);

private:
    // Впомогательные методы
    QList<CandleData> parseCandles(const QByteArray& data);
    void fetchCandles(const QString& symbol, Interval interval, int limit);

    // REST
    QNetworkAccessManager* m_networkManager;

    // WebSocket
    QWebSocket* m_webSocket;
    bool m_isRealtimeConnected = false;

    // Состояние
    QString m_symbol;
    Interval m_interval;
    qint64 m_lastCandleTime = 0;

 };
#endif // EXCHANGECLIENT_H
