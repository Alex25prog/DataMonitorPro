#include "TradingChartManager.h"
#include <QtCharts/QCandlestickSet>
#include <QDebug>

// используем пространство имен Qt Charts
//QT_CHARTS_USE_NAMESPACE

TradingChartManager::TradingChartManager(QObject *parent)
    : QObject(parent)
    , m_series(new QCandlestickSeries(this))
{
    m_series->setName("Price");
    // Используем целочисленные значения между строк
    m_series->setIncreasingColor(QColor(0x26, 0xa6, 0x9a)); // #26a69a
    m_series->setDecreasingColor(QColor(0xef, 0x53, 0x50)); // #ef5350
    m_series->setBodyWidth(0.7);
    m_series->setMaximumColumnWidth(30);
    m_series->setMinimumColumnWidth(5);
}

TradingChartManager::~TradingChartManager()
{
    // QCandlestickSeries очистится автоматически через parent
}

void TradingChartManager::updateSeries(CandleModel* model)
{
    if (!model) {
        qDebug() << "TradingChartManager: model is null";
        return;
    }
    int count = model->rowCount();
    qDebug() << "TradingChartManager: updating series with" << model->rowCount() << "candles";

    // Очищаем старые свечи
    m_series->clear();

    if (count == 0) {
        qDebug() << "TradingChartManager: no candles to display";
        return;
    }

    // Создаем свечи напрямую в С++
    for (int i = 0; i < model->rowCount(); ++i) {
        CandleData candle = model->getCandle(i);

        QCandlestickSet* set = new QCandlestickSet(
            candle.open,
            candle.high,
            candle.low,
            candle.close,
            candle.openTime,
            this
        );

        m_series->append(set);
    }

    qDebug() << "TradingChartManager: series now has" << m_series->count() << "candles";
}

void TradingChartManager::clearSeries()
{
    m_series->clear();
    qDebug() << "TradingChartManager: series cleared";
}