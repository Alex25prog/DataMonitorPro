#ifndef EXCHANGECLIENT_H
#define EXCHANGECLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
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
    void loadHistory(const QString& symbol, Interval interval, int limit = 100);
    void startRealtimeUpdates(const QString& symbol, Interval interval);
    void stopRealtimeUpdates();
    void setSymbol(const QString& symbol) { m_symbol = symbol; }
    QString symbol() const { return m_symbol;}


signals:
    void candlesLoaded(const QList<CandleData>& candles);
    void newCandle(const CandleData& candle);
    void errorOccurred(const QString& error);

private slots:
    void onReplyFinished(QNetworkReply* reply);
    void onRealtimeUpdate();

private:
    QNetworkAccessManager* m_manager;
    QTimer* m_realtimeTimer;
    QString m_symbol;
    Interval m_interval;
    bool m_isRealtimeRunning = false;
    qint64 m_lastCandleTime = 0;

    // Вспомогательные методы
    QString intervalToString(Interval interval) const;
    QList<CandleData> parseCandles(const QByteArray& data);
    void fetchCandles(const QString& symbol, Interval interval, int limit);
 };
#endif // EXCHANGECLIENT_H
