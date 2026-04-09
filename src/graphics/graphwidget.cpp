#include "graphwidget.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QVariantMap>

GraphWidget::GraphWidget(QQuickItem *parent) : QQuickPaintedItem(parent), m_plot(nullptr)
{
    //Настройки внешнего вида виджета
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    qDebug() << "GraphWidget constructor this  =" << this;
    setupPlot();//Принудительно создаем график

    //Этот таймер нужен для плавного обновления графика

}

GraphWidget::~GraphWidget()
{
    qDebug() << "GraphWidget destructor this =" << this;
    if (m_plot) {
        delete m_plot;
        m_plot = nullptr;
    }

}

void GraphWidget::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    setupPlot();
}

void GraphWidget::setupPlot()
{
    qDebug() << "setup called";
    if (m_plot) return;

        //создаем виджет QCustomPlot
        m_plot = new QCustomPlot();

        //настройка фона
        m_plot->setBackground(QColor(30 ,30, 30));

        //Легенда
        m_plot->legend->setVisible(true);
        m_plot->legend->setBrush(QBrush(QColor(40, 40, 40)));
        m_plot->legend->setTextColor(Qt::white);
        m_plot->legend->setBorderPen(QPen(Qt::gray));

        //настройка осей

        m_plot->xAxis->setLabel("Time");
        m_plot->yAxis->setLabel("Value");

        //Настройка внешнего вида

        m_plot->xAxis->setBasePen(QPen(Qt::white));
        m_plot->yAxis->setBasePen(QPen(Qt::white));
        m_plot->xAxis->setTickLabelColor(Qt::white);
        m_plot->yAxis->setTickLabelColor(Qt::white);
        m_plot->xAxis->setLabelColor(Qt::white);
        m_plot->yAxis->setLabelColor(Qt::white);

        //График температуры(красный)
        m_plot->addGraph();
        m_plot->graph(0)->setName("Temperature");
        m_plot->graph(0)->setPen(QPen(QColor(255, 80, 80), 2));
        m_plot->graph(0)->setBrush(QBrush(QColor(255, 80, 80, 60)));//Полупрозрачная заливка

        //График давления (синий)
        m_plot->addGraph();
        m_plot->graph(1)->setName("Pressure");
        m_plot->graph(1)->setPen(QPen(QColor(80, 150, 255), 2));
        m_plot->graph(1)->setBrush(QBrush(QColor(80, 150, 255, 60)));

        //График влажности(зеленый)
        m_plot->addGraph();
        m_plot->graph(2)->setName("Numidity");
        m_plot->graph(2)->setPen(QPen(QColor(80, 255, 80), 2));
        m_plot->graph(2)->setBrush(QBrush(QColor(80, 255, 80, 60)));

        m_plot->replot();
        qDebug() << "GraphWidget: setup with 3 graphs";


    }


void GraphWidget::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size(). isValid() && !m_plot) {
        setupPlot(); //Создаем график, когда станет известен его размер
    }
    if (m_plot && newGeometry.size().isValid()) {
        //Изменяем размер контейнера при изменении размера родителя
        m_plot->setGeometry(QRect(0, 0, newGeometry.width(), newGeometry.height()));
    }
}

void GraphWidget::paint(QPainter *painter)
{
    //Этот метод нужен для корректной работы QQuickPaintedItem
    //График будет рисоваться с помощью библиотеки QCustomPlot.
    qDebug() << "paint called, m_plot =" << m_plot
             << "size=" << (m_plot ? m_plot->size() : QSize())
             << "rect=" << boundingRect();

    if (m_plot) {
        QImage image(m_plot->size(), QImage::Format_ARGB32);
        QPainter imagePainter(&image);
        m_plot->render(&imagePainter);
        painter->drawImage(0, 0, image);
    }else{
        painter->fillRect(0, 0, width(), height(), Qt::red);
    }
}

QVariantMap GraphWidget::lastPoint() const{

    // Пока не используем
    return QVariantMap();
}

void GraphWidget::setLastPoint(const QVariantMap &point)
{
    double timestamp = point ["timestamp"]. toDouble();
    double value = point["value"]. toDouble();
    QString type = point["type"]. toString();//Добавим тип данных

    qDebug() << "GraphWidget received (this=" << this << "):" << type << timestamp << value;

    //Нормализация для отображения на одном графике
    double normalizedValue = value;
    if (type == "pressure")
    {
        normalizedValue = value / 10; //1000 = 100
    }
    else if (type == "humidity"){

        normalizedValue = value; //60 = 60
    }
    else if (type == "temperature"){

        normalizedValue = value * 10; //6 = 60
    }

    //Добавляем в соответствующий вектор

    if (type == "temperature") {


    m_tempX.append(timestamp);
    m_tempY.append(normalizedValue);

    //Для ограничения количства точек (например 100)

    const int maxPoints = 100;
    if (m_tempX.size() > maxPoints){
        m_tempX.remove(0, m_tempX.size() - maxPoints);
        m_tempY.remove(0, m_tempY.size() - maxPoints);
    }

}
    else if (type == "pressure"){
        m_pressX.append(timestamp);
        m_pressY.append(normalizedValue);
        const int maxPoints = 100;
        if (m_pressX.size() > maxPoints){
            m_pressX.remove(0, m_pressX.size() - maxPoints);
            m_pressY.remove(0, m_pressY.size() - maxPoints);
      }
}
else if (type == "humidity"){
    m_humX.append(timestamp);
    m_humY.append(normalizedValue);
    const int maxPoints = 100;
    if (m_humX.size() > maxPoints){
        m_humX.remove(0, m_humX.size() - maxPoints);
        m_humY.remove(0, m_humY.size() - maxPoints);

    }
}

updateGraph();
update();//дополнительная перерисовка
emit lastPointChanged();
}
void GraphWidget::updateGraph()
{
    if (!m_plot) return;
    qDebug() << "updateGraph this=" << this << "m_plot=" << m_plot;



    //Устанавливаем данные для каждого графика
    m_plot->graph(0)->setData(m_tempX, m_tempY);
    m_plot->graph(1)->setData(m_pressX, m_pressY);
    m_plot->graph(2)->setData(m_humX, m_humY);

    //Автоматическое масштабирование осей
    m_plot->rescaleAxes();

    //Небольшой отступ по краям

    m_plot->xAxis->setRange(m_plot->xAxis->range().lower, m_plot->xAxis->range().upper + 1);

    //перерисовываем
    m_plot->replot();
    //Принудительно запрашиваем перерисовку QQuickpaintedItem

    update();//Запрос перерисовки QQuickPaintedItem

    qDebug() << "udateGraph: called update(rect)";

    /**qDebug() << "Graph updated - temp:" << m_tempX.size()
             << "press:" << m_pressX.size()
             << "hum:" << m_humX.size();*/
}