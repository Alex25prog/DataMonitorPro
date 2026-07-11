#ifndef CANDLEMODEL_H
#define CANDLEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "../models/CandleData.h"


class CandleModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        OpenTimeRole = Qt::UserRole + 1,
        OpenRole,
        HighRole,
        LowRole,
        CloseRole,
        VolumeRole,
        CloseTimeRole

    };

    explicit CandleModel(QObject *parent = nullptr);

    // QAbstractListMosel методы
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;


    // Методы для работы с данными
    Q_INVOKABLE void setCandles(const QList<CandleData>& candles);
    Q_INVOKABLE void addCandle(const CandleData& candle);
    Q_INVOKABLE void clear();
    Q_INVOKABLE CandleData getCandle(int index) const;
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void countChanged();
    void dataAdded();

private:
    QList<CandleData> m_candles;
};

#endif // CANDLEMODEL_H
