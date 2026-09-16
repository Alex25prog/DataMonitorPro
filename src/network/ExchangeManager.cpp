#include "ExchangeManager.h"
#include <QDebug>

ExchangeManager::ExchangeManager(QObject *parent)
    : QObject(parent)
{
}

void ExchangeManager::registerAdapter(const QString& name, IExchangeAdapter* adapter)
{
    if (!adapter) return;

    m_adapters[name] = adapter;
    qDebug() << "ExchangeManager: registered adapter" << name;

    // Первый зарегистрированный адаптер становится активным по умолчанию.
    if (m_currentName.isEmpty()) {
        m_currentName = name;
        connectAdapterSignals(adapter);
    }
}

void ExchangeManager::switchExchange(const QString& name)
{
    if (!m_adapters.contains(name)) {
        qDebug() << "ExchangeManager: unknown exchange" << name;
        return;
    }
    if (name == m_currentName) {
        return;
    }

    IExchangeAdapter* old = currentAdapter();
    if (old) {
        // Останавливаем realtime у биржи, с которой уходим — иначе её
        // WebSocket продолжит молча работать в фоне без всякой пользы.
        old->stopRealtime();
        disconnectAdapterSignals(old);
    }

    m_currentName = name;
    connectAdapterSignals(currentAdapter());

    qDebug() << "ExchangeManager: switched to" << name;
    emit exchangeChanged(name);
}

IExchangeAdapter* ExchangeManager::currentAdapter() const
{
    return m_adapters.value(m_currentName, nullptr);
}

void ExchangeManager::connectAdapterSignals(IExchangeAdapter* adapter)
{
    if (!adapter) return;

    connect(adapter, &IExchangeAdapter::candlesLoaded,
            this, &ExchangeManager::candlesLoaded);
    connect(adapter, &IExchangeAdapter::newCandleTick,
            this, &ExchangeManager::newCandleTick);
    connect(adapter, &IExchangeAdapter::errorOccurred,
            this, &ExchangeManager::errorOccurred);
    connect(adapter, &IExchangeAdapter::connectionStatusChanged,
            this, &ExchangeManager::connectionStatusChanged);
    connect(adapter, &IExchangeAdapter::tradeReceived,
            this, &ExchangeManager::tradeReceived);
}

void ExchangeManager::disconnectAdapterSignals(IExchangeAdapter* adapter)
{
    if (!adapter) return;
    disconnect(adapter, nullptr, this, nullptr);
}

void ExchangeManager::loadMarket(const QString& symbol, Interval interval, int limit)
{
    if (auto* a = currentAdapter()) {
        a->loadMarket(symbol, interval, limit);
    }
}

void ExchangeManager::startRealtime()
{
    if (auto* a = currentAdapter()) {
        a->startRealtime();
    }
}

void ExchangeManager::stopRealtime()
{
    if (auto* a = currentAdapter()) {
        a->stopRealtime();
    }
}

bool ExchangeManager::isRealtimeConnected() const
{
    auto* a = currentAdapter();
    return a && a->isRealtimeConnected();
}

QString ExchangeManager::symbol() const
{
    auto* a = currentAdapter();
    return a ? a->symbol() : QString();
}

Interval ExchangeManager::interval() const
{
    auto* a = currentAdapter();
    return a ? a->interval() : Interval::H1;
}