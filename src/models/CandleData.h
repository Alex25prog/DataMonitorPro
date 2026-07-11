#ifndef CANDLEDATA_H
#define CANDLEDATA_H

#include <QObject>
#include <QDateTime>

// Структура свечи
struct CandleData
{
    qint64 openTime;      // Unix timestamp в миллисекундах
    double open;
    double high;
    double low;
    double close;
    double volume;
    qint64 closeTime;     // Unix timestamp в миллисекундах
    bool isClosed = true; // Свеча закрыта или еще формируется
};

#endif // CANDLEDATA_H
