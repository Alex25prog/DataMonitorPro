#include "TradingChartManager.h"
#include <QtCharts/QCandlestickSet>
#include <QDebug>

TradingChartManager::TradingChartManager(QObject *parent)
    : QObject(parent)
    , m_updating(false)
{
    qDebug() << "TradingChartManager created";
}

TradingChartManager::~TradingChartManager()
{
    qDebug() << "TradingChartManager destroyed";
}

void TradingChartManager::attachSeries(QCandlestickSeries* series)
{
    if (!series) {
        qWarning() << "TradingChartManager: Attempted to attach a null series!";
        return;
    }

    if (m_series.data() == series) {
        qDebug() << "TradingChartManager: This series is already attached.";
        return;
    }

    if (m_series) {
        qDebug() << "TradingChartManager: Detaching old series and attaching a new one.";
        clearSeries();
    }

    m_series = series;

    // Отключаем анимации для стабильности
    if (m_series) {
        // В Qt 6.8 это свойство может отсутствовать у QCandlestickSeries
        // m_series->setAnimationOptions(QChart::NoAnimation);
    }

    qDebug() << "TradingChartManager: QML series successfully attached!";
}

void TradingChartManager::updateSeries(CandleModel* model)
{
    if (m_updating) {
        qDebug() << "TradingChartManager: Already updating, skipping...";
        return;
    }

    if (!m_series) {
        qDebug() << "TradingChartManager: Series not attached!";
        return;
    }

    if (!model) {
        qDebug() << "TradingChartManager: Model is null!";
        return;
    }

    m_updating = true;


    m_series->clear();

    int count = model->rowCount();
    qDebug() << "TradingChartManager: Updating series with" << count << "candles";

    if (count > 0) {
        QList<QCandlestickSet*> newSets;
        newSets.reserve(count);

        for (int i = 0; i < count; ++i) {
            CandleData candle = model->getCandle(i);

            QCandlestickSet* set = new QCandlestickSet(
                candle.open,
                candle.high,
                candle.low,
                candle.close,
                static_cast<qreal>(candle.openTime),
                m_series.data()
                );
            newSets.append(set);
        }

        m_series->append(newSets);
        qDebug() << "TradingChartManager: Appended" << newSets.size() << "candles in batch";
    }

    qDebug() << "TradingChartManager: series now has" << m_series->count() << "candles";

    m_updating = false;
    emit seriesUpdated();
}

void TradingChartManager::clearSeries()
{
    if (!m_series) {
        return;
    }


    // clear() УЖЕ УДАЛЯЕТ все sets, НЕ НАДО ДЕЛАТЬ qDeleteAll

    m_series->clear();

    qDebug() << "TradingChartManager: series cleared";
}

void TradingChartManager::updateLastCandle(const CandleData& candle)
{
    if (!m_series) {
        qDebug() << "TradingChartManager: series not attached!";
        return;
    }

    if (m_updating) {
        qDebug() << "TradingChartManager: Already updating, skipping...";
        return;
    }

    m_updating = true;

    auto sets = m_series->sets();

    if (sets.isEmpty()) {
        // Первая свеча
        QCandlestickSet* set = new QCandlestickSet(
            candle.open,
            candle.high,
            candle.low,
            candle.close,
            static_cast<qreal>(candle.openTime),
            m_series.data()
            );
        m_series->append(set);
        qDebug() << "TradingChartManager: first candle added via WebSocket";
        emit seriesUpdated();
        m_updating = false;
        return;
    }

    QCandlestickSet* lastSet = sets.last();

    if (lastSet->timestamp() != static_cast<qreal>(candle.openTime)) {
        // Новая свеча
        QCandlestickSet* newSet = new QCandlestickSet(
            candle.open,
            candle.high,
            candle.low,
            candle.close,
            static_cast<qreal>(candle.openTime),
            m_series.data()
            );
        m_series->append(newSet);
        qDebug() << "TradingChartManager: new candle added via WebSocket";
    } else {
        // Обновляем существующую
        lastSet->setOpen(candle.open);
        lastSet->setHigh(candle.high);
        lastSet->setLow(candle.low);
        lastSet->setClose(candle.close);
        lastSet->setTimestamp(static_cast<qreal>(candle.openTime));
        qDebug() << "TradingChartManager: last candle updated via WebSocket";
    }

    emit seriesUpdated();
    m_updating = false;
}