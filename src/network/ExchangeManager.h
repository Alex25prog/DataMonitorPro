#ifndef EXCHANGEMANAGER_H
#define EXCHANGEMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include "IExchangeAdapter.h"

// Держит набор адаптеров бирж и перенаправляет вызовы/сигналы к тому,
// который сейчас выбран активным. MainController и QML работают только
// с этим классом — им не нужно знать, сколько бирж подключено и какая
// из них активна прямо сейчас.
class ExchangeManager : public QObject
{
    Q_OBJECT

public:
    explicit ExchangeManager(QObject *parent = nullptr);

    // Владение адаптером НЕ передаётся сюда — адаптер должен быть создан
    // с тем же родителем (обычно MainController), что и раньше.
    void registerAdapter(const QString& name, IExchangeAdapter* adapter);

    Q_INVOKABLE void switchExchange(const QString& name);
    QStringList availableExchanges() const { return m_adapters.keys(); }
    QString currentExchangeName() const { return m_currentName; }
    IExchangeAdapter* currentAdapter() const;
    IExchangeAdapter* adapter(const QString& name) const { return m_adapters.value(name, nullptr); }

    // Делегирование к активному адаптеру
    void loadMarket(const QString& symbol, Interval interval, int limit = 100);
    void startRealtime();
    void stopRealtime();
    bool isRealtimeConnected() const;
    QString symbol() const;
    Interval interval() const;

signals:
    void candlesLoaded(const QList<CandleData>& candles);
    void tradeReceived(double price, qint64 tradeTimeMs);
    void newCandleTick(const CandleData& candle);
    void errorOccurred(const QString& error);
    void connectionStatusChanged(bool connected);
    void exchangeChanged(const QString& name);

private:
    void connectAdapterSignals(IExchangeAdapter* adapter);
    void disconnectAdapterSignals(IExchangeAdapter* adapter);

    QMap<QString, IExchangeAdapter*> m_adapters;
    QString m_currentName;
};

#endif // EXCHANGEMANAGER_H