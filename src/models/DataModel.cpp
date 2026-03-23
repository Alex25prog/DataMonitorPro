#include "DataModel.h"

DataModel::DataModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

DataModel::~DataModel()
{
}

int DataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_filteredData.size();
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredData.size())
        return QVariant();

    const DataPoint& point = m_filteredData[index.row()];

    switch (role) {
    case TimestampRole:
        return point.timestamp().toString("yyyy-MM-dd hh:mm:ss");
    case TypeRole:
        return point.type();
    case ValueRole:
        return point.value();
    case UnitRole:
        return point.unit();
    case StringRole:
        return point.toString();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DataModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimestampRole] = "timestamp";
    roles[TypeRole] = "type";
    roles[ValueRole] = "value";
    roles[UnitRole] = "unit";
    roles[StringRole] = "string";
    return roles;
}

void DataModel::addDataPoint(const DataPoint& point)
{
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(point);
    endInsertRows();
    
    // Если точка проходит фильтры, добавляем в отфильтрованный список
    if (passesFilters(point)) {
        int filteredIndex = m_filteredData.size();
        beginInsertRows(QModelIndex(), filteredIndex, filteredIndex);
        m_filteredData.append(point);
        endInsertRows();
    }
    
    emit countChanged();
    emit dataAdded(point);
}

void DataModel::clear()
{
    beginResetModel();
    m_data.clear();
    m_filteredData.clear();
    endResetModel();
    emit countChanged();
}

DataPoint DataModel::getDataPoint(int index) const
{
    if (index < 0 || index >= m_filteredData.size())
        return DataPoint();
    return m_filteredData[index];
}

void DataModel::setTypeFilter(const QString& type)
{
    m_typeFilter = type;
    applyFilters();
}

void DataModel::setTimeRange(const QDateTime& from, const QDateTime& to)
{
    m_timeFrom = from;
    m_timeTo = to;
    applyFilters();
}

void DataModel::resetFilters()
{
    m_typeFilter.clear();
    m_timeFrom = QDateTime();
    m_timeTo = QDateTime();
    applyFilters();
}

void DataModel::applyFilters()
{
    beginResetModel();
    m_filteredData.clear();
    
    for (const DataPoint& point : m_data) {
        if (passesFilters(point)) {
            m_filteredData.append(point);
        }
    }
    
    endResetModel();
    emit countChanged();
}

bool DataModel::passesFilters(const DataPoint& point) const
{
    // Фильтр по типу
    if (!m_typeFilter.isEmpty() && point.type() != m_typeFilter) {
        return false;
    }
    
    // Фильтр по времени
    if (m_timeFrom.isValid() && point.timestamp() < m_timeFrom) {
        return false;
    }
    if (m_timeTo.isValid() && point.timestamp() > m_timeTo) {
        return false;
    }
    
    return true;
}