#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "../core/DataPoint.h"

class DataModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        TimestampRole = Qt::UserRole + 1,
        TypeRole,
        ValueRole,
        UnitRole,
        StringRole
    };

    explicit DataModel(QObject *parent = nullptr);
    ~DataModel();

    // Методы QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Методы для работы с данными
    Q_INVOKABLE void addDataPoint(const DataPoint& point);
    Q_INVOKABLE void clear();
    Q_INVOKABLE DataPoint getDataPoint(int index) const;
    
    // Фильтрация
    Q_INVOKABLE void setTypeFilter(const QString& type);
    Q_INVOKABLE void setTimeRange(const QDateTime& from, const QDateTime& to);
    Q_INVOKABLE void resetFilters();

signals:
    void countChanged();
    void dataAdded(const DataPoint& point);

private:
    QVector<DataPoint> m_data;           // все данные
    QVector<DataPoint> m_filteredData;   // отфильтрованные данные
    QString m_typeFilter;                 // фильтр по типу
    QDateTime m_timeFrom;                 // фильтр по времени (начало)
    QDateTime m_timeTo;                   // фильтр по времени (конец)
    
    void applyFilters();
    bool passesFilters(const DataPoint& point) const;
};

#endif // DATAMODEL_H