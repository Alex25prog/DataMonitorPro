#include "GraphWindow.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QVariantMap>
#include <QTimer>

GraphWindow::GraphWindow(QWidget *parent) : QWidget(parent), m_plot(nullptr)
    , m_startTime(0)

{
    setupPlot(); //Создаем график в конструкторе
    setWindowTitle("DataMonitor Pro - Real-Time Graph");
    resize(900, 500);

}

GraphWindow::~GraphWindow()
{
    //qDebug() << "GraphWidget destructor this =" << this;
    if (m_plot) delete m_plot;
        //m_plot = nullptr;
    }



/*void GraphWidget::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    //setupPlot();
}
*/
void GraphWindow::setupPlot()
{
        //qDebug() << "setup called";
        if (m_plot) return;

        //создаем виджет QCustomPlot
        m_plot = new QCustomPlot();

        //настройка фона "темный"
        m_plot->setBackground(QColor(30 ,30, 30));


        //настройка осей

        m_plot->xAxis->setLabel("Time (seconds)");
        m_plot->yAxis->setLabel("Value (normalized)");
        m_plot->xAxis->setBasePen(QPen(Qt::white, 2));
        m_plot->yAxis->setBasePen(QPen(Qt::white, 2));
        m_plot->xAxis->setTickLabelColor(Qt::white);
        m_plot->yAxis->setTickLabelColor(Qt::white);
        m_plot->xAxis->setLabelColor(Qt::white);
        m_plot->yAxis->setLabelColor(Qt::white);

        //График температуры(красный)
        m_plot->addGraph();
        m_plot->graph(0)->setName("Temperature");
        m_plot->graph(0)->setPen(QPen(QColor(255, 80, 80), 2));
        m_plot->graph(0)->setBrush(Qt::NoBrush);//Без заливки

        //График давления (синий)
        m_plot->addGraph();
        m_plot->graph(1)->setName("Pressure");
        m_plot->graph(1)->setPen(QPen(QColor(80, 150, 255), 2));
        m_plot->graph(1)->setBrush(Qt::NoBrush);

        //График влажности(зеленый)
        m_plot->addGraph();
        m_plot->graph(2)->setName("Numidity");
        m_plot->graph(2)->setPen(QPen(QColor(80, 255, 80), 2));
        m_plot->graph(2)->setBrush(Qt::NoBrush);

        //Легенда
        m_plot->legend->setVisible(true);
        m_plot->legend->setBrush(QBrush(QColor(40, 40, 40)));
        m_plot->legend->setTextColor(Qt::white);
        m_plot->legend->setBorderPen(QPen(Qt::gray));

        //инициализируем графики одной точкой (чтобы они ожили)
        m_plot->graph(0)->addData(0, 0);
        m_plot->graph(1)->addData(0, 0);
        m_plot->graph(2)->addData(0, 0);

        //Layout
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(m_plot);
        setLayout(layout);

        m_plot->replot();
        qDebug() << "GraphWidget: setup comlete";

    }

void GraphWindow::addPoint(double timestamp, double value, const QString& type)
{
    //Нормализация времени
    if (m_startTime == 0) {
        m_startTime = timestamp;
    }
    double relativeTime = (timestamp - m_startTime) / 1000.0;

    //Нормализация значений (0-100)
    double norm = value;
    if (type == "pressure")
    {
        norm = value / 10.26;
    }
    else if (type == "humidity"){

        norm = value;
    }
    else if (type == "temperature"){

        norm = value * 10;
    }

    //Добавляем в соответствующий вектор

    if (type == "temperature") {


    m_tempX.append(relativeTime);
    m_tempY.append(norm);

    //Для ограничения количства точек (например 100)
    if (m_tempX.size() > 100){
        m_tempX.remove(0, m_tempX.size() - 100);
        m_tempY.remove(0, m_tempY.size() - 100);
    }

} else if (type == "pressure"){
        m_pressX.append(relativeTime);
        m_pressY.append(norm);
        //const int maxPoints = 100;
        if (m_pressX.size() > 100){
            m_pressX.remove(0, m_pressX.size() - 100);
            m_pressY.remove(0, m_pressY.size() - 100);
      }
} else if (type == "humidity"){
    m_humX.append(relativeTime);
    m_humY.append(norm);
    //const int maxPoints = 100;
    if (m_humX.size() > 100){
        m_humX.remove(0, m_humX.size() - 100);
        m_humY.remove(0, m_humY.size() - 100);

    }
}

updateGraph();

//Принудительная перерисовка
if (m_plot) {
    m_plot->replot();
    update();
  }
}
void GraphWindow::clearGraphs()
{
    //Очищаем векторы
    m_tempX.clear(); m_tempY.clear();
    m_pressX.clear(); m_pressY.clear();
    m_humX.clear(); m_humY.clear();
    m_startTime = 0;
    updateGraph();
    qDebug() << "GraphWindow: cleared";
}

void GraphWindow::updateGraph()
{

    if (!m_plot) return;

    //Устанавливаем данные для каждого графика
    m_plot->graph(0)->setData(m_tempX, m_tempY);
    m_plot->graph(1)->setData(m_pressX, m_pressY);
    m_plot->graph(2)->setData(m_humX, m_humY);
    \
    //Автоматическое масштабирование осей
    m_plot->rescaleAxes();
    m_plot->replot();

   // Небольшой отступ по оси X
    double maxX = 0;
    if (!m_tempX.isEmpty()) maxX = qMax(maxX, m_tempX.last());
    if (!m_pressX.isEmpty()) maxX = qMax(maxX, m_pressX.last());
    if (!m_humX.isEmpty()) maxX = qMax(maxX, m_humX.last());
    if (maxX > 0) {
        m_plot->xAxis->setRange(0, maxX + 1);
    }

    //перерисовываем
    m_plot->replot();
}
void GraphWindow::forceRepaint()
{
    if (m_plot) {
        updateGraph();
        m_plot->replot();
        update();
    }
}
