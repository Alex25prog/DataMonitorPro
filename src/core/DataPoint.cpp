#include "DataPoint.h"

DataPoint::DataPoint()
    : m_timestamp(QDateTime::currentDateTime())
    , m_value(0.0)
{
}

DataPoint::DataPoint(const QDateTime& timestamp, const QString& type, double value, const QString& unit)
    : m_timestamp(timestamp)
    , m_type(type)
    , m_value(value)
    , m_unit(unit)
{
}

QString DataPoint::toString() const
{
    return QString("%1 | %2 | %3 %4")
        .arg(m_timestamp.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(m_type)
        .arg(m_value)
        .arg(m_unit);
}

bool DataPoint::isValid() const
{
    return !m_type.isEmpty() && m_timestamp.isValid();
}