#include "chartview.h"

ChartView::ChartView(QWidget *parent): QChartView(parent)
{
    this->setRubberBand(QChartView::RectangleRubberBand);
    this->setCursor(Qt::OpenHandCursor);
}

ChartView::ChartView(QChart* chart, QWidget *parent): QChartView(chart, parent)
{
    m_chart = chart;
    this->setRubberBand(QChartView::RectangleRubberBand);
    this->setCursor(Qt::OpenHandCursor);
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    QPoint ang_data = event->angleDelta();
    if(ang_data.y() > 0) this->chart()->zoomIn();
    else if(ang_data.y() < 0) this->chart()->zoomOut();
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << event->type();
    oldx = event->globalX();
    oldy = event->globalY();
    chartmoving = true;
    this->setCursor(Qt::ClosedHandCursor);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << event->type();
    chartmoving = false;
    this->setCursor(Qt::OpenHandCursor);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(chartmoving){
        int dX = event->globalX() - oldx,
            dY = event->globalY() - oldy;
        qDebug() << dX << dY;
        m_chart->scroll(-dX, dY);
        oldx = event->globalX();
        oldy = event->globalY();
    }
    qDebug() << event->type();
}
