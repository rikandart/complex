#include "chartview.h"
#include <QElapsedTimer>

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
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
//    this->setUpdatesEnabled(true);
}

void ChartView::setAxisAndRange(QValueAxis *axisX, QValueAxis *axisY)
{
    m_axisX = axisX;
    m_axisY = axisY;
    minX = m_axisX->min();
    maxX = m_axisX->max();
    minY = m_axisY->min();
    maxY = m_axisY->max();
    zoomedminX = minX;
    zoomedmaxX = maxX;
    zoomedminY = minY;
    zoomedmaxY = maxY;
    double stepX = (maxX - minX + 1)/double(GRIDCOUNT),
           stepY = (maxY - minY + 1)/double(GRIDCOUNT);
    grid[0] = minX;
    grid[GRIDCOUNT] = minY;
//    qDebug() << "nodes" << grid[0] << grid[GRIDCOUNT];
    for(unsigned i = 1; i < GRIDCOUNT+1; i++){
        grid[i] = grid[i-1] + stepX;
        grid[GRIDCOUNT+i] = grid[GRIDCOUNT+i-1] + stepY;
//        qDebug() << "nodes" << grid[i] << grid[GRIDCOUNT+i];
    }
}

void ChartView::resetAxis(QValueAxis *axis)
{
    if(axis == m_axisX){
        axis->setMin(minX);
        axis->setMax(maxX);
        zoomedminX = minX;
        zoomedmaxX = maxX;
    }
    if(axis == m_axisY){
        axis->setMin(minY);
        axis->setMax(maxY);
        zoomedminY = minY;
        zoomedmaxY = maxY;
    }
}

void ChartView::resetZoomedAxis(QValueAxis *axis)
{
    if(axis == m_axisX){
        axis->setMin(zoomedminX);
        axis->setMax(zoomedmaxX);
    }
    if(axis == m_axisY){
        axis->setMin(zoomedminY);
        axis->setMax(zoomedmaxY);
    }
}

void ChartView::getZoomedAxes()
{
    zoomedminX = m_axisX->min();
    zoomedmaxX = m_axisX->max();
    zoomedminY = m_axisY->min();
    zoomedmaxY = m_axisY->max();
}

void ChartView::drawCross(const QPointF point)
{
   // m_chart->addSeries();
}

// поиск ближайшей точки к позиции мышки
QPointF ChartView::nearestNode(float x, float y)
{
    QPointF point;
    {
        QPointF value = m_chart->mapToValue(QPointF(x, y));
//        qDebug() << "value" << value;
        x = value.x(), y = value.y();
    }
//    qDebug() << x << y;
    float abs_diff[2*GRIDCOUNT+1];
    for(unsigned i = 0; i < GRIDCOUNT+1; i++){
        abs_diff[i] = abs(x-grid[i]);
        abs_diff[GRIDCOUNT+i] = abs(y-grid[GRIDCOUNT+i]);
//        qDebug() << "abs diff" << abs_diff[i] << abs_diff[GRIDCOUNT+i];
    }
    float min_x = abs_diff[0], min_y = abs_diff[GRIDCOUNT];
    unsigned i_x = 0, i_y = GRIDCOUNT;
    for (unsigned i = 1; i < GRIDCOUNT+1; i++){
        if(abs_diff[i] < min_x){
            min_x = abs_diff[i];
//            qDebug() << "min_x found" << min_x;
            i_x = i;
        }
        if(abs_diff[GRIDCOUNT+i] < min_y){
            min_y = abs_diff[GRIDCOUNT+i];
//            qDebug() << "min_y found" << min_y;
            i_y = GRIDCOUNT+i;
        };
    }
    point.setX(grid[i_x]);
    point.setY(grid[i_y]);
//    qDebug() << point;
    return m_chart->mapToPosition(point);
}

void ChartView::wheelEvent(QWheelEvent *event)
{
//    QElapsedTimer timer1, timer2;
//    timer1.start();
    QPoint ang_data = event->angleDelta();
    QRectF area = m_chart->plotArea();
//    timer2.start();
    QPointF nearest = nearestNode(event->x(), event->y());
//    qDebug() << "nearest" << nearest;
//    qDebug() << "Elapsed nearestNode() time" << timer2.elapsed() << timer2.nsecsElapsed();
    bool nozooms = false;
    auto zoomIn = [&nearest, this](QRectF& area, Axis type, unsigned short& zoomcount, qreal coef, bool zoom)->void{
        if(zoom){
            if(zoomcount < ZoomMax){
                QPointF value = m_chart->mapToValue(nearest);
                switch(type){
                case Axis::x:
                    area.setWidth(area.width()*coef);
                    area.moveCenter(QPoint(nearest.x(), nearest.y()));
                    zoomcount++;
                    break;
                case Axis::y:
                    area.setHeight(area.height()*coef);
                    area.moveCenter(QPoint(area.center().x(), nearest.y()));
                    zoomcount++;
                    // если есть зум по иксу, то смещение осуществляется только по иксу
                    if(!anchorX){
                        float curview = m_axisX->max() - m_axisX->min();
                        m_axisX->setRange(value.x()-curview/2, value.x()+curview/2);
                    }
                    break;
                }
            }
        }
    };
    auto zoomOut = [&nearest, this](QRectF& area, Axis type, unsigned short& zoomcount, qreal coef, bool zoom)->void{
        if(zoom){
            if(zoomcount > 0){
                switch(type){
                case Axis::x:
                    area.setWidth(area.width()*coef);
                    area.moveCenter(nearest);
                    zoomcount--;
                    break;
                case Axis::y:
                    area.setHeight(area.height()*coef);
                    area.moveCenter(nearest);
                    zoomcount--;
                    break;
                }
            } else {
                switch (type) {
                case Axis::x:
                    resetAxis(m_axisX);
                    break;
                case Axis::y:
                    resetAxis(m_axisY);
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
                if(!(anchorX || anchorY)) anchorX = anchorY = nozooms = true;
                zoomIn(area, Axis::x, zoomcountX, coef, anchorX);
                zoomIn(area, Axis::y, zoomcountY, coef, anchorY);
                if(nozooms) anchorX = anchorY = nozooms = false;

            } else if(ang_data.y() < 0){
                if(!(anchorX || anchorY)) anchorX = anchorY = nozooms = true;
                zoomOut(area, Axis::x, zoomcountX, coef, anchorX);
                zoomOut(area, Axis::y, zoomcountY, coef, anchorY);
                if(nozooms) anchorX = anchorY = nozooms = false;
            }
            m_chart->zoomIn(area);
            // перемещение курсора
            areacenter = m_chart->plotArea().center();
            QCursor::setPos(this->mapToGlobal(QPoint(areacenter.x(), areacenter.y())));
            getZoomedAxes();
    }
//    qDebug() << "Elapsed wheelEvent() time" << timer1.elapsed() << timer1.nsecsElapsed();
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    // перетаскивание графика
    oldx = event->x();
    oldy = event->y();
    switch(event->button()){
        case Qt::LeftButton:
        // перемещение графика
             chartmoving = true;
             this->setCursor(Qt::ClosedHandCursor);
        break;
        case Qt::RightButton:
        // сделать зум по выделенной области
        break;
    }
}

void ChartView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // даблклик центрирует график и сбрасывает зум
    zoomcountX = 0;
    zoomcountY = 0;
    m_chart->zoomReset();
    resetAxis(m_axisX);
    resetAxis(m_axisY);
    // emit doubleClicked();
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    chartmoving = xmoving = ymoving = false;
    this->setCursor(Qt::OpenHandCursor);
    getZoomedAxes();
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
    this->scene()->invalidate(this->sceneRect());
    if(chartmoving){
        int curx = event->x(),
            cury = event->y();
        int dX = curx - oldx,
            dY = cury - oldy;
        // если нажат shift
        if(anchorY){
            if(xmoving) dY = 0;
            if(ymoving) dX = 0;
            if(abs(dX) > 3){
                dY = 0;
                xmoving = true;
                resetZoomedAxis(m_axisY);
            }
            if(abs(dY) > 2){
                dX = 0;
                ymoving = true;
                resetZoomedAxis(m_axisX);
            }
        }
        m_chart->scroll(-dX, dY);
        oldx = curx;
        oldy = cury;
    }
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    Qt::Key key = Qt::Key(event->key());
    if (key == Qt::Key_Control) anchorX = true;
    if (key == Qt::Key_Shift) anchorY = true;
    if (key == Qt::Key_Right || Qt::Key_Left) emit arrowPressed(key);
}

void ChartView::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Control) anchorX = false;
    if (key == Qt::Key_Shift) anchorY = false;
}

void ChartView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QRectF area = m_chart->plotArea();
    if(area.contains(m_mousePos)){
        QRectF hint (m_mousePos.x() - hintShift - hintWidth, m_mousePos.y() - hintShift - hintHeight,
                     hintWidth, hintHeight);
        QRectF hintBorder (m_mousePos.x() - hintShift - hintWidth - 1, m_mousePos.y() - hintShift - hintHeight-1,
                           hintWidth+1, hintHeight+1);
        painter->setPen(QPen(QColor(122, 130, 144), 1));
        painter->drawLine(area.left(), m_mousePos.y(), area.right(), m_mousePos.y());
        painter->drawLine(m_mousePos.x(), area.top(), m_mousePos.x(), area.bottom());
        painter->drawRect(hintBorder);
        painter->fillRect(hint, QBrush(Qt::white));
        QPointF value = m_chart->mapToValue(m_mousePos);
        painter->setPen(QPen(Qt::black, 10));
        // текст рисуется вверх от заданно точки расположения
        painter->drawText(hint.topLeft() + QPointF(3, 12), "x: " + QString().sprintf("%4.4f",value.x()));
        painter->drawText(hint.topLeft() + QPointF(3, 25), "y: " + QString().sprintf("%4.4f",value.y()));
    }
}
