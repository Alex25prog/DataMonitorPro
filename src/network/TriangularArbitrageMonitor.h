#ifndef TRIANGULARARBITRAGEMONITOR_H
#define TRIANGULARARBITRAGEMONITOR_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QPointer>

/*
 * Мониторинг треугольного арбитража на ОДНОЙ бирже (Binance spot).
   Цикл: USDT -> BTC -> ETH -> USDT (прямой) и USDT -> ETH -> BTC -> USDT (обратный).

   ВАЖНО: этот класс только СЧИТАЕТ и ПОКАЗЫВАЕТ расчётную прибыль в реальном
   времени по трём парам (BTCUSDT, ETHUSDT, ETHBTC) через bookTicker-поток
   (лучшие bid/ask, обновляется на каждое изменение стакана, а не раз в
   секунду как поток свечей). Реальные ордера НЕ выставляются — это
   сознательное решение: для реального исполнения нужны API-ключи с правом
   торговли, обработка частичных исполнений и проскальзывания — отдельная,
   гораздо более рискованная задача.
*/

class TriangularArbitrageMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)

    Q_PROPERTY(QString symbolA READ symbolA CONSTANT) // BTCUSD
    Q_PROPERTY(QString symbolB READ symbolB CONSTANT) // ETHUSDT
    Q_PROPERTY(QString symbolC READ symbolC CONSTANT) // ETHBTC

    Q_PROPERTY(double bidA READ bidA NOTIFY pricesChanged)
    Q_PROPERTY(double askA READ askA NOTIFY pricesChanged)
    Q_PROPERTY(double bidB READ bidB NOTIFY pricesChanged)
    Q_PROPERTY(double askB READ askB NOTIFY pricesChanged)
    Q_PROPERTY(double bidC READ bidC NOTIFY pricesChanged)
    Q_PROPERTY(double askC READ askC NOTIFY pricesChanged)

    Q_PROPERTY(double feePercent READ feePercent WRITE setFeePercent NOTIFY feePercentChanged)

    Q_PROPERTY(double forwardProfitPercent READ forwardProfitPercent NOTIFY profitChanged)
    Q_PROPERTY(double reverseProfitPercent READ reverseProfitPercent NOTIFY profitChanged)
    Q_PROPERTY(QString lastUpdateTime READ lastUpdateTime NOTIFY profitChanged)


public:
    explicit TriangularArbitrageMonitor(QObject *parent = nullptr);
    ~TriangularArbitrageMonitor();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    bool isRunning() const { return m_isRunning; }

    QString symbolA() const { return m_symbolA; }
    QString symbolB() const { return m_symbolB; }
    QString symbolC() const { return m_symbolC; }

    double bidA() const { return m_bidA; }
    double askA() const { return m_askA; }
    double bidB() const { return m_bidB; }
    double askB() const { return m_askB; }
    double bidC() const { return m_bidC; }
    double askC() const { return m_askC; }

    double feePercent() const { return m_feePercent; }
    void setFeePercent(double value);

    double forwardProfitPercent() const { return m_forwardProfitPercent; }
    double reverseProfitPercent() const { return m_reverseProfitPercent; }
    QString lastUpdateTime() const { return m_lastUpdateTime; }

signals:
    void runningChanged();
    void pricesChanged();
    void feePercentChanged();
    void profitChanged();
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onTextMessageReceived(const QString& message);
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    void recomputeProfit();

    QPointer<QWebSocket> m_webSocket;
    bool m_isRunning = false;

    // Классический треугольник на Binance: USDT как база, BTC/ETH как ноги
    QString m_symbolA = "BTCUSDT";
    QString m_symbolB = "ETHUSDT";
    QString m_symbolC = "ETHBTC";

    double m_bidA = 0.0, m_askA = 0.0;
    double m_bidB = 0.0, m_askB = 0.0;
    double m_bidC = 0.0, m_askC = 0.0;

    // % комиссии за одну сделку (типичный Binance spot taker fee - 0.1%).
    // За полный цикл комиссия применяется трижды
    double m_feePercent = 0.1;

    double m_forwardProfitPercent = 0.0;
    double m_reverseProfitPercent = 0.0;
    QString m_lastUpdateTime;

};

#endif // TRIANGULARARBITRAGEMONITOR_H
