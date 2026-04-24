#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

//#include <QQuickPaintedItem>
#include <QWidget>
#include <QVBoxLayout>
#include <QPointer>
#include <QVariantMap>
#include <qquickpainteditem.h>
#include "qcustomplot.h"

class GraphWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWindow(QWidget *parent = nullptr);
    ~GraphWindow();

    void addPoint(double timestamp, double value, const QString& type);
    void clearGraphs();
    void forceRepaint();


    private:

        QCustomPlot* m_plot;
        QVector<double> m_tempX, m_tempY; //Векторы для хранения разных типов данных
        QVector<double> m_pressX, m_pressY;//Давление
        QVector<double> m_humX, m_humY;//Влажность
        double m_startTime;
        void setupPlot(); // Настройка графика
        void updateGraph();// обновление графика новыми данными

};

#endif
