#ifndef TRADINGCHARTMANAGER_H
#define TRADINGCHARTMANAGER_H

#include <QObject>
#include <QPointer>
#include <QtCharts/QChart>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QCandlestickSet>
#include "../models/CandleModel.h"

//QT_BEGIN_NAMESPACE
//class QCandlestickSeries;
//class QCandlestickSet;
//QT_END_NAMESPACE

QT_USE_NAMESPACE

class TradingChartManager : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(QCandlestickSeries* candlestickSeries READ candlestickSeries CONSTANT)

public:
    explicit TradingChartManager(QObject *parent = nullptr);
    ~TradingChartManager();

    //QCandlestickSeries* candlestickSeries() const { return m_series; }

    Q_INVOKABLE void attachSeries(QCandlestickSeries* series); // Метод для привязки серии из QML
    Q_INVOKABLE void updateSeries(CandleModel* model);
    Q_INVOKABLE void clearSeries();
    Q_INVOKABLE void updateLastCandle(const CandleData& candle);

signals:
    void seriesUpdated(); //Сигнал для QML

private:
   // QPointer сам станет nullptr, если QML удалит объект графика
    QPointer<QCandlestickSeries> m_series;//  Безопасный указатель
    bool m_updating; // Защита от рекурсии
};

#endif // TRADINGCHARTMANAGER_H
