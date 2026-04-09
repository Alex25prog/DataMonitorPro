#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QQuickPaintedItem>
#include <QPointer>
#include <QVariantMap>
#include "qcustomplot.h"

class GraphWidget : public QQuickPaintedItem
{
    Q_OBJECT
    //Это свойство будет видно из QML
    Q_PROPERTY(QVariantMap lastPoint READ lastPoint WRITE setLastPoint NOTIFY lastPointChanged )

public:
    explicit GraphWidget(QQuickItem *parent = nullptr);
    ~GraphWidget();

    //Метод для рисования (обязателен для QQuickPaintedItem)
    void paint(QPainter *painter) override;

    //Свойство для получения новых данных
    QVariantMap lastPoint() const;
    void setLastPoint(const QVariantMap &point);

    void componentComplete() override;


    signals:
    void lastPointChanged();

    protected:
        //Этот метод будет вызываться, когда размер виджета изменится
        void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    private:
        QPointer<QCustomPlot> m_plot;// Сам виджет с графиком
        void setupPlot(); // Настройка графика
        void updateGraph();// обновление графика новыми данными


        QVector<double> m_tempX, m_tempY; //Векторы для хранения разных типов данных
        QVector<double> m_pressX, m_pressY;//Давление
        QVector<double> m_humX, m_humY;//Влажность

};

#endif