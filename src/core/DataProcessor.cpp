#include "DataProcessor.h"

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
{
}

DataProcessor::~DataProcessor()
{
}

void DataProcessor::processDataPoint(const DataPoint& point)
{
    // Пока просто передаем сигнал дальше
    emit dataProcessed(point);
    
    // TODO: добавить расчет статистики
}