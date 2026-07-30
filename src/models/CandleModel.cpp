#include "CandleModel.h"
#include <QDebug>

CandleModel::CandleModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int CandleModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_candles.size();
}

QVariant CandleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_candles.size())
        return QVariant();

    const CandleData& candle = m_candles[index.row()];

    switch (role) {
    case OpenTimeRole:
        return QDateTime::fromMSecsSinceEpoch(candle.openTime).toString("yyyy-MM-dd hh:mm:ss");

    case OpenRole:
        return candle.open;

    case HighRole:
        return candle.high;

    case LowRole:
        return candle.low;

    case CloseRole:
        return candle.close;

    case VolumeRole:
        return candle.volume;

    case CloseTimeRole:
        return QDateTime::fromMSecsSinceEpoch(candle.closeTime).toString("yyyy-MM-dd hh:mm:ss");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CandleModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[OpenTimeRole] = "openTime";
    roles[OpenRole] = "open";
    roles[HighRole] = "high";
    roles[LowRole] = "low";
    roles[CloseRole] = "close";
    roles[VolumeRole] = "volume";
    roles[CloseTimeRole] = "closeTime";
    return roles;
}

void CandleModel::addCandle(const CandleData& candle)
{
    beginInsertRows(QModelIndex(), m_candles.size(), m_candles.size());
    m_candles.append(candle);
    endInsertRows();
    emit countChanged();
    emit dataAdded();
}

void CandleModel::clear()
{
    beginResetModel();
    m_candles.clear();
    endResetModel();
    emit countChanged();
}

CandleData CandleModel::getCandle(int index) const
{
    if (index < 0 || index >= m_candles.size()) {
        return CandleData();
        qDebug() << "CandleModel: invalid index" << index << ", size:" << m_candles.size();
    }
    return m_candles[index];
}

QVariantMap CandleModel::get(int row) const
{
    QVariantMap map;

    if (row < 0 || row >= m_candles.size())
        return map;

    const CandleData &c = m_candles[row];

    map["openTime"] = c.openTime;
    map["closeTime"] = c.closeTime;
    map["open"] = c.open;
    map["high"] = c.high;
    map["low"] = c.low;
    map["close"] = c.close;
    map["volume"] = c.volume;
    map["isClosed"] = c.isClosed;

    return map;
}

void CandleModel::setCandles(const QList<CandleData>& candles)
{
    beginResetModel();
    m_candles = candles;
    endResetModel();
    emit countChanged();
    qDebug() << "CandleMosel: set" << m_candles.size() << "candles";
}

void CandleModel::addOrUpdateCandle(const CandleData& candle)
{
    if (m_candles.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_candles.append(candle);
        endInsertRows();
        emit countChanged();
        emit dataAdded();
        return;
    }

    CandleData& last = m_candles.last();
    if (last.openTime == candle.openTime) {
        // Обновляем последнюю свечу
        if (candle.isClosed) {
            last = candle;
        } else {
            last.high = candle.high;
            last.low = candle.low;
            last.close = candle.close;
            last.volume = candle.volume;
        }
        QModelIndex idx = index(m_candles.size() - 1);
        emit dataChanged(idx, idx);
        emit countChanged();
        return;
    }

    // Новая свеча
    beginInsertRows(QModelIndex(), m_candles.size(), m_candles.size());
    m_candles.append(candle);
    endInsertRows();
    emit countChanged();
    emit dataAdded();
}