#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include "DataPoint.h"

class DataProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DataProcessor(QObject *parent = nullptr);
    ~DataProcessor();

    void processDataPoint(const DataPoint& point);

signals:
    void dataProcessed(const DataPoint& point);
    void statisticsUpdated(double min, double max, double avg, int count);

private:
    // TODO: добавить поля для статистики
};

#endif // DATAPROCESSOR_H