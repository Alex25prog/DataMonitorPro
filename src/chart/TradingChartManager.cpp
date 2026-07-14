#include "TradingChartManager.h"
#include <QtCharts/QCandlestickSet>
#include <QDebug>

// используем пространство имен Qt Charts
//QT_CHARTS_USE_NAMESPACE

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
    // Защита от передачи пустышки
    if (!series) {
        qWarning() << "TradingChartManager: Attemted to attach a null series!";
        return;
    }

    // Защита от дублирования: проверяем, не этот ли самый график уже привязан
    // Используем data() для безопасного сравнения адресов
    if (m_series.data() == series) {
        qDebug() << "TradingChartManager: This series is already attached.";
        return;
    }

    // Если привязывается новая серия, очищаем старую (если она еще жива)
    if (m_series) {
        qDebug() << "TradingChartManager: Detaching old series and attaching a new one.";
        clearSeries();
    }
    m_series = series;
    qDebug() << "TradingChartManager: QML series successfully attached!";
}

void TradingChartManager::updateSeries(CandleModel* model)
{
    // Защита от рекурсивного вызова
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

    // Очищаем старые данные. clear() сам безопасно уведомит UI
    auto oldSets = m_series->sets();
    m_series->clear();
    qDeleteAll(oldSets);

    int count = model->rowCount();
    qDebug() << "TradingChartManager: Updating series with" << count << "candles";

    if (count > 0) {
        // Создаем временный список для свечей
        QList<QCandlestickSet*> newSets;
        newSets.reserve(count); // Резервируем память для производительности

    // Создаем свечи напрямую в С++
    for (int i = 0; i <count; ++i) {
        CandleData candle = model->getCandle(i);

        // Используем m_series.data() чтобы передать чистый QObject*
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

    // Для мгновенной отрисовки всех свечей без зависаний и крашей
    m_series->append(newSets);

    qDebug() << "TradingChartManager: Appended" << newSets.size() << "candles in batch";
}

    qDebug() << "TradingChartManager: series now has" << m_series->count() << "candles";

    m_updating = false;
    emit seriesUpdated(); // QML сделает updateAxes() и перерисовку
}

void TradingChartManager::clearSeries()
{
    if (!m_series) return;

    auto oldSets = m_series->sets();
    m_series->clear();
    qDeleteAll(oldSets);
    //m_series->blockSignals(false);

    qDebug() << "TradingChartManager: series cleared";
}

void TradingChartManager::updateLastCandle(const CandleData& candle)
{
    if (!m_series){
        qDebug() << "TradingChartManager: series not attached!";
        return;
    }

    auto sets = m_series->sets();

    if (sets.isEmpty()) {
        // Если нет свечей — добавляем первую
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
}