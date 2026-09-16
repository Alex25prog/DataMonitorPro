#ifndef IEXCHANGEADAPTER_H
#define IEXCHANGEADAPTER_H

#include <QObject>
#include <QString>
#include <QList>
#include "../models/CandleData.h"

/* Общие интервалы свечей. Каждый адаптер сам переводит их в формат своей
 * биржи (Binance: "1h", Bybit: "60" и т.д (конвертация внутри адаптера)
 * снаружи все работает с одними и теми же значениями
 */
enum class Interval {
    M1, M5, M15, M30, H1, H4, D1
};

/* Контракт, который должна реализовать Каждая биржа. MainController и QML
 * работают только с этим интерфейсом - им все равно, Binance это, Bybit
 * или кто-то еще, пока сигналы и методы ведут себя одинаково
 */
class IExchangeAdapter : public QObject
{
    Q_OBJECT

public:
    explicit IExchangeAdapter(QObject *parent = nullptr) : QObject(parent)
    {}
    virtual ~IExchangeAdapter() = default;

    // Человекочитаемое имя биржи для UI (селектор биржи в QML)
    virtual QString exchangeName() const = 0;

    // Загрузка истории по REST - только истории, WebSocket не трогает
    virtual void loadMarket(const QString& symbol, Interval interval, int limit = 100) = 0;

    // Явное управление realtime-подпиской (WebSocket на свечи)
    virtual void startRealtime() = 0;
    virtual void stopRealtime() = 0;

    virtual bool isRealtimeConnected() const = 0;
    virtual QString symbol() const = 0;
    virtual Interval interval() const = 0;

signals:
    void candlesLoaded(const QList<CandleData>& candles);
    void newCandleTick(const CandleData& candle);
    void errorOccurred(const QString& error);
    void connectionStatusChanged(bool connected);
    void tradeReceived(double price, qint64 tradeTimeMs); //Быстрый поток цены (отдельные сделки биржи)
                                                          //обновляется на каждую реальную сделку, а не раз в секунду
                                                          //как поток свечей
};

#endif // IEXCHANGEADAPTER_H
