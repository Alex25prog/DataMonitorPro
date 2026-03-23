#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <QDateTime>
#include <QString>

class DataPoint
{
public:
    DataPoint();
    DataPoint(const QDateTime& timestamp, const QString& type, double value, const QString& unit = "");

    // Геттеры (методы для получения данных)
    QDateTime timestamp() const { return m_timestamp; }
    QString type() const { return m_type; }
    double value() const { return m_value; }
    QString unit() const { return m_unit; }

    // Сеттеры (методы для установки данных)
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }
    void setType(const QString& type) { m_type = type; }
    void setValue(double value) { m_value = value; }
    void setUnit(const QString& unit) { m_unit = unit; }

    // Вспомогательные методы
    QString toString() const;  // для отладки
    bool isValid() const;      // проверка валидности

private:
    QDateTime m_timestamp;     // время получения данных
    QString m_type;            // тип данных (температура, давление и т.д.)
    double m_value;            // значение
    QString m_unit;            // единица измерения (C, hPa, %)
};

#endif // DATAPOINT_H