#include "chartview.h"

ChartView::ChartView(QWidget *parent): QChartView(parent)
{
    this->setRubberBand(QChartView::RectangleRubberBand);
    this->setCursor(Qt::OpenHandCursor);
    this->setFocus();
}

ChartView::ChartView(QChart* chart, QWidget *parent): QChartView(chart, parent)
{
    m_chart = chart;
    this->setRubberBand(QChartView::RectangleRubberBand);
    this->setCursor(Qt::OpenHandCursor);
    this->setFocus();
}

void ChartView::resetZoom()
{
    zoomcountX = 0;
    zoomcountY = 0;
}

void ChartView::setAxisAndRange(QValueAxis *axisX, QValueAxis *axisY)
{
    m_axisX = axisX;
    m_axisY = axisY;
    minX = m_axisX->min();
    maxX = m_axisX->max();
    minY = m_axisY->min();
    maxY = m_axisY->max();
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    QPoint ang_data = event->angleDelta();
    QRectF area = m_chart->plotArea();
    bool nozooms = false;
    auto zoomIn = [&event, this](QRectF& area, Zoom type, unsigned short zoomcount, qreal coef, bool zoom)->void{
        if(zoom){
            if(zoomcount < ZoomMax){
                QPointF value = m_chart->mapToValue(QPointF(event->x(), event->y()));
                switch(type){
                case Zoom::x:
                    area.setWidth(area.width()*coef);
                    zoomcount++;
                    area.moveCenter(QPoint(event->x(), event->y()));
                    break;
                case Zoom::y:
                    area.setHeight(area.height()*coef);
                    area.moveCenter(QPoint(area.center().x(), event->y()));
                    zoomcount++;
                   // area.moveCenter(QPoint(event->x(), event->y()));
                    m_axisX->setRange(value.x()-m_axisX->max()/2, value.x()+m_axisX->max()/2);
                    break;
                }
            }
        }
    };
    auto zoomOut = [](QRectF& area, Zoom type, unsigned short zoomcount, qreal coef, bool zoom)->void{
        if(zoom){
            if(zoomcount > 0){
                switch(type){
                case Zoom::x:
                    area.setWidth(area.width()*coef);
                    zoomcount--;
                    break;
                case Zoom::y:
                    area.setHeight(area.height()*coef);
                    zoomcount--;
                    break;
                }
            }
        }
    };
    if(area.contains(QPoint(event->x(), event->y()))){
            QPointF areacenter = area.center();
            qreal coef = ang_data.y() > 0 ? 1.0/ZoomCoef : ZoomCoef;
            if(ang_data.y() > 0){
                // если control или shift не были нажаты,
                // то зумируем по обеим осям
                if(!(zoomX || zoomY)) zoomX = zoomY = nozooms = true;
                zoomIn(area, Zoom::x, zoomcountX, coef, zoomX);
                zoomIn(area, Zoom::y, zoomcountY, coef, zoomY);
                if(nozooms) zoomX = zoomY = nozooms = false;

            } else if(ang_data.y() < 0){
                if(!(zoomX || zoomY)) zoomX = zoomY = nozooms = true;
                zoomOut(area, Zoom::x, zoomcountX, coef, zoomX);
                zoomOut(area, Zoom::y, zoomcountY, coef, zoomY);
                if(nozooms) zoomX = zoomY = nozooms = false;
            }
            qDebug() << area;
            m_chart->zoomIn(area);
            // перемещение курсора
            areacenter = m_chart->plotArea().center();
            QCursor::setPos(this->mapToGlobal(QPoint(areacenter.x(), areacenter.y())));
    }

}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    // перетаскивание графика
    oldx = event->globalX();
    oldy = event->globalY();
    chartmoving = true;
    this->setCursor(Qt::ClosedHandCursor);
}

void ChartView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // даблклик центрирует график и сбрасывает зум
    zoomcountX = 0;
    zoomcountY = 0;
    m_chart->zoomReset();
    m_axisX->setMin(minX);
    m_axisX->setMax(maxX);
    // emit doubleClicked();
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    chartmoving = false;
    this->setCursor(Qt::OpenHandCursor);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(chartmoving){
        int curx = event->globalX(),
            cury = event->globalY();
        int dX = curx - oldx,
            dY = cury - oldy;
        m_chart->scroll(-dX, dY);
        dXsum += dX, dYsum = dY;
        oldx = curx;
        oldy = cury;
    }
    QString str = "x: " + QString::number(event->x()) + " y: " + QString::number(event->y());
    m_chart->setTitle(str);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) zoomX = true;
    if (event->key() == Qt::Key_Shift) zoomY = true;
}

void ChartView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) zoomX = false;
    if (event->key() == Qt::Key_Shift) zoomY = false;
}
