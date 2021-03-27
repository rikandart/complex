#include "chartview.h"
#include <QElapsedTimer>

ChartView::ChartView(QWidget *parent): QChartView(parent)
{
    this->setRubberBand(QChartView::RectangleRubberBand);
    this->setCursor(Qt::OpenHandCursor);
    this->setFocus();
}

ChartView::ChartView(QChart* chart, ChartType type, QWidget *parent):
    QChartView(chart, parent), m_chart(chart), chartType(type)
{
    this->setRubberBand(QChartView::RectangleRubberBand);
    this->setFocus();
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setRubberBand(QChartView::RectangleRubberBand);
    m_prevMousePos = QPointF(-1, -1);
    QTimer::singleShot(timer_msec, this, SLOT(updateView()));
}

void ChartView::setAxisAndRange(QValueAxis *axisX, QValueAxis *axisY)
{
    if(axisX) m_axisX = axisX;
    if(axisY) m_axisY = axisY;
    Q_ASSERT(m_axisX != nullptr && m_axisY != nullptr);
    minX = m_axisX->min();
    maxX = m_axisX->max();
    minY = m_axisY->min();
    maxY = m_axisY->max();
    zoomedminX = minX;
    zoomedmaxX = maxX;
    zoomedminY = minY;
    zoomedmaxY = maxY;
    buildGrid();
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

void ChartView::buildGrid()
{
    // сетку необходимо сделать в отсчетах для приближения именно в нужном месте
    if(!m_axisX || !m_axisY) return;
    double  sampling_rate = Cuza::get().getFd(),
            l_minX = (m_axisX->min() >= minX ? m_axisX->min() : minX)*sampling_rate,
            l_maxX = (m_axisX->max() <= maxX ? m_axisX->max() : maxX)*sampling_rate,
            l_minY = (m_axisY->min() >= minY ? m_axisY->min() : minY),
            l_maxY = (m_axisY->max() <= maxY ? m_axisY->max() : maxY),
            stepX = (l_maxX - l_minX + 1)/double(GRIDCOUNT),
            stepY = (l_maxY - l_minY + 1)/double(GRIDCOUNT);
    grid[0] = l_minX;
    grid[GRIDCOUNT] = l_minY;
//    qDebug() << "nodes" << grid[0] << grid[GRIDCOUNT];
    for(unsigned i = 1; i < GRIDCOUNT+1; i++){
        grid[i] = grid[i-1] + stepX;
        grid[GRIDCOUNT+i] = grid[GRIDCOUNT+i-1] + stepY;
//        qDebug() << "nodes" << grid[i] << grid[GRIDCOUNT+i];
    }
}

void ChartView::centerVertically()
{
    if (!(zoomedmaxY + zoomedminY)) return;
    // в случае с графиком амплитудного спектра спукаем его вниз на 0
    if(chartType == ChartType::chAMP){
        ((QValueAxis*)(m_chart->axes()[1]))->setMax(
                    ((QValueAxis*)(m_chart->axes()[1]))->max() -
                    ((QValueAxis*)(m_chart->axes()[1]))->min());
        ((QValueAxis*)(m_chart->axes()[1]))->setMin(0);
        return;
    }
    // флаг спуска или подъема графика
    bool direction = abs(zoomedmaxY) > abs(zoomedminY);
    while(direction ? (zoomedmaxY-zoomedminY) < 2*zoomedmaxY : (zoomedminY-zoomedmaxY) > 2*zoomedminY){
        double coef = 0.0;
        if(abs(zoomedmaxY+zoomedminY) > 2) coef = 1;
        else{
            coef = 0.01;
            // если разница между величинами уже несущественна,
            // то присваиваем минимальный противоположный максимальному и выходим из цикла
            if(abs(2*zoomedmaxY - (zoomedmaxY-zoomedminY)) < 0.02) {
                zoomedminY = -zoomedmaxY;
                break;
            }
        }
        if(!direction) coef = -coef;
        zoomedmaxY -= coef;
        zoomedminY -= coef;
    }
    m_axisY->setMax(zoomedmaxY);
    m_axisY->setMin(zoomedminY);
    getZoomedAxes();
}

// поиск ближайшей точки к позиции мышки
QPointF ChartView::nearestNode(float x, float y)
{
    QPointF point;
    {
        QPointF value = m_chart->mapToValue(QPointF(x, y));
//        qDebug() << "value" << value;
        // сводим полученные значения к отсчетам для согласованности
        // с массивом сетки
        x = value.x()*Cuza::get().getFd(), y = value.y();
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
    point.setX(grid[i_x]/Cuza::get().getFd());
    point.setY(grid[i_y]);
//    qDebug() << point;
    return m_chart->mapToPosition(point);
}

void ChartView::mainwinResized()
{
    buildGrid();
}

void ChartView::receiveEvent(InputEvent type, QInputEvent* event)
{
    Q_ASSERT(event != nullptr);
    receiving = true;
    switch (type) {
    case InputEvent::MWHEEL:
        this->wheelEvent(static_cast<QWheelEvent*>(event));
        break;
    case InputEvent::MPRESS:
        this->mousePressEvent(static_cast<QMouseEvent*>(event));
        break;
    case InputEvent::MDOUBLECLICK:
        this->mouseDoubleClickEvent(static_cast<QMouseEvent*>(event));
        break;
    case InputEvent::MRELEASE:
        this->mouseReleaseEvent(static_cast<QMouseEvent*>(event));
        break;
    case InputEvent::MMOVE:
        this->mouseMoveEvent(static_cast<QMouseEvent*>(event));
        break;
    case InputEvent::KPRESS:
        this->keyPressEvent(static_cast<QKeyEvent*>(event));
        break;
    case InputEvent::KRELEASE:
        this->keyReleaseEvent(static_cast<QKeyEvent*>(event));
        break;
    }
    receiving = false;
}

void ChartView::receivePointsVecSize(unsigned size, ChartType type)
{
    if(type == chartType) m_serVecSize = size;
}

void ChartView::updateView()
{
    if(this->size() != oldSize){
        buildGrid();
        oldSize = this->size();
    }
    this->scene()->invalidate(this->sceneRect());
    QTimer::singleShot(timer_msec, this, SLOT(updateView()));
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    zoomChart(QPointF(event->x(), event->y()), event->angleDelta().y());
    if(!receiving) emit transmitEvent(InputEvent::MWHEEL, event);
}

void ChartView::zoomChart(QPointF pos, int ang_data, QRectF zoomArea)
{
    QRectF area = m_chart->plotArea();
    QPointF nearest = nearestNode(pos.x(), pos.y());
    bool noanchors = false;
    auto zoomIn = [&nearest, this](QRectF& area, Axis type, double& zoomcount, qreal coef, bool zoom)->void{
        if(zoom){
            if(zoomcount < ZoomMax){
                QPointF value = m_chart->mapToValue(nearest);
                switch(type){
                case Axis::x:
                    area.setWidth(area.width()*coef);
                    area.moveCenter(QPointF(nearest.x(), nearest.y()));
                    zoomcount+=coef;
                    break;
                case Axis::y:
                    area.setHeight(area.height()*coef);
                    area.moveCenter(QPointF(area.center().x(), nearest.y()));
                    zoomcount+=coef;
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
    auto zoomOut = [&nearest, this](QRectF& area, Axis type, double& zoomcount, qreal coef, bool zoom)->void{
        if(zoom){
            if(zoomcount > 0){
                switch(type){
                case Axis::x:
                    area.setWidth(area.width()*coef);
                    area.moveCenter(nearest);
                    zoomcount-=1/coef;
                    break;
                case Axis::y:
                    area.setHeight(area.height()*coef);
                    area.moveCenter(nearest);
                    zoomcount-=1/coef;
                    break;
                }
            } else {
                switch (type) {
                case Axis::x:
                    resetAxis(m_axisX);
                    zoomcount = 0;
                    break;
                case Axis::y:
                    resetAxis(m_axisY);
                    zoomcount = 0;
                    break;
                }
            }
        }
    };
    if(area.contains(pos)){
            QPointF areacenter = area.center();
            qreal coef = ang_data > 0 ? 1.0/ZoomCoef : ZoomCoef;
            if(ang_data > 0 && zoomArea == QRectF()){
                // если control или shift не были нажаты,
                // то зумируем по обеим осям
                if(!(anchorX || anchorY)) anchorX = anchorY = noanchors = true;
                zoomIn(area, Axis::x, zoomcountX, coef, anchorX);
                zoomIn(area, Axis::y, zoomcountY, coef, anchorY);
                if(noanchors) anchorX = anchorY = noanchors = false;

            } else if(ang_data < 0 && zoomArea == QRectF()){
                if(!(anchorX || anchorY)) anchorX = anchorY = noanchors = true;
                zoomOut(area, Axis::x, zoomcountX, coef, anchorX);
                zoomOut(area, Axis::y, zoomcountY, coef, anchorY);
                if(noanchors) anchorX = anchorY = noanchors = false;
            }
            // 250321 доделать до суммирования зума через коэффициент
            if(zoomArea != QRectF()){
                if (zoomcountX < ZoomMax)
                    zoomcountX += log10(zoomArea.width()/area.width())/log10(coef)*coef;
                else zoomArea.setWidth(area.width());
                if(zoomcountY < ZoomMax)
                    zoomcountY += log10(zoomArea.height()/area.height())/log10(coef)*coef;
                else zoomArea.setHeight(area.height());
                area = zoomArea;
            }
            m_chart->zoomIn(area);
            // перемещение курсора
            if(!receiving) {
                areacenter = m_chart->plotArea().center();
                QCursor::setPos(this->mapToGlobal(QPoint(areacenter.x(), areacenter.y())));
            }
            buildGrid();
            getZoomedAxes();
    }
}

void ChartView::zoomChart(QRectF zoomArea, bool zoomIn)
{
    zoomChart(zoomArea.center(), zoomIn ? 1 : -1, zoomArea);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    // перетаскивание графика
    oldx = event->x();
    oldy = event->y();
    switch(event->button()){
        case Qt::LeftButton:
            if(m_prevMousePos == QPointF(-1,-1)){
                 // перемещение графика
                 chartmoving = true;
                 if(!receiving && set_label && m_chart->plotArea().contains(m_mousePos)){
                     if(labels.size() == lab_count) labels.clear();
                     labels.append(nearestValue(m_chart->mapToValue(m_mousePos)));
                 }
            }
        break;
        case Qt::RightButton:
            // сделать зум по выделенной области
            m_prevMousePos = m_mousePos;
            if(receiving) mouse_pressed = true;
        break;
    }
    if(!receiving) emit transmitEvent(InputEvent::MPRESS, event);
}

void ChartView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(!anchorY) {// даблклик центрирует график и сбрасывает зум
        zoomcountX = 0;
        zoomcountY = 0;
        m_chart->zoomReset();
        resetAxis(m_axisX);
        resetAxis(m_axisY);
    } else centerVertically(); // центрирование по высоте (shift + double click)
    buildGrid();
    if(set_label && this->hasFocus()) labels.clear();
    if(!receiving) emit transmitEvent(InputEvent::MDOUBLECLICK, event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    chartmoving = xmoving = ymoving = mouse_pressed = false;
    this->setCursor(Qt::OpenHandCursor);
    getZoomedAxes();
    if(event->button() == Qt::RightButton){
        if(m_zoomRect.topLeft().x() > m_zoomRect.bottomRight().x()
                || m_zoomRect.topLeft().y() > m_zoomRect.bottomRight().y()){
            QPointF tmp = m_zoomRect.topLeft();
            m_zoomRect.setTopLeft(m_zoomRect.bottomRight());
            m_zoomRect.setBottomRight(tmp);
        }
        m_prevMousePos = QPointF(-1,-1);
        zoomChart(m_zoomRect);
        m_zoomRect = QRectF();
    }
    // иначе будет продолжаться бесконечно
    if(!receiving) emit transmitEvent(InputEvent::MRELEASE, event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(!receiving) this->setFocus();
    m_mousePos = event->pos();
    bool contains = m_chart->plotArea().contains(m_mousePos);
    if(chartmoving) this->setCursor(Qt::ClosedHandCursor);
    else if(contains && m_prevMousePos == QPointF(-1,-1)) this->setCursor(Qt::OpenHandCursor);
    else this->setCursor(Qt::ArrowCursor);
    if(chartmoving && contains){
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
        buildGrid();
    }
    if(!receiving) emit transmitEvent(InputEvent::MMOVE, event);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    Qt::Key key = Qt::Key(event->key());
    if (key == Qt::Key_Control) anchorX = true;
    if (key == Qt::Key_Shift)   anchorY = true;
    if (key == Qt::Key_Alt)     set_label = true;
    if ((key == Qt::Key_Right || key == Qt::Key_Left) && !receiving){
        labels.clear();
        emit arrowPressed(key);
    }
    if(!receiving) emit transmitEvent(InputEvent::KPRESS, event);
}

void ChartView::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Control) anchorX = false;
    if (key == Qt::Key_Shift)   anchorY = false;
    if (key == Qt::Key_Alt)     set_label = false;
    if(!receiving) emit transmitEvent(InputEvent::KRELEASE, event);
}

void ChartView::drawForeground(QPainter *painter, const QRectF &rect)
{
    drawAxesLabels(painter);
    if(m_chart->plotArea().contains(m_mousePos)) drawHint(painter, m_mousePos);
    if (labels.size() != 0)
        for(uchar i = 0; i < labels.size(); i++) drawHint(painter, labels[i], true);
    if(m_prevMousePos != QPointF(-1, -1) && m_chart->plotArea().contains(m_prevMousePos)){
        drawZoomRect(m_prevMousePos, painter);
    }
}

void ChartView::drawHint(QPainter *painter, QPointF pos, bool label)
{
    QRectF area = m_chart->plotArea();
    QPointF value = !label ? nearestValue(m_chart->mapToValue(pos)) :  pos;
    pos = m_chart->mapToPosition(value, m_chart->series()[0]);
    painter->setPen(QPen(QColor(122, 130, 144), hintBorderWidth));
    if(area.contains(pos)) painter->drawLine(pos.x(), area.top(), pos.x(), area.bottom());
    // рисуем метку, если нет приема перерсовки и не нажимается кнопка мыши
    // для того, чтобы метки не отображались на графиках, принимающих qt-сигналы
    if(receiving) return;
    if(area.contains(this->mapFromGlobal(QCursor::pos())) || label){
        if(area.contains(pos))
            painter->drawLine(area.left(), pos.y(), area.right(), pos.y());
        QRectF hint;
        calcHint(pos, hint, area);
        // если расчитанная метка может влезть в площадь графика
        if(area.contains(hint)){
            painter->setPen(QPen(QColor(122, 130, 144), hintBorderWidth));
            painter->fillRect(hint, QBrush(Qt::white));
            painter->drawRect(hint);
            painter->setPen(QPen(Qt::black, 10));
            // текст рисуется вверх от заданной точки расположения
            painter->drawText(hint.topLeft() + QPointF(3.0, 12), "x: " + QString().sprintf("%4.4f",value.x()));
            painter->drawText(hint.topLeft() + QPointF(3.0, 25), "y: " + QString().sprintf("%4.4f",value.y()));
        }
    }
    if(labels.size() == lab_count){
        QRectF dHint;
        calcHint(m_chart->mapToPosition(labels[lab_count-1]), dHint, area, 1, 2);
        if(area.contains(dHint)){
            painter->fillRect(dHint, QBrush(Qt::white));
            painter->setPen(QPen(QColor(122, 130, 144), hintBorderWidth));
            painter->drawRect(dHint);
            painter->setPen(QPen(Qt::black, 10));
            QPointF value = labels[lab_count-1] - labels[lab_count-2];
            painter->drawText(dHint.topLeft() + QPointF(3.0, 12), "dx: " + QString().sprintf("%4.4f", fabs(value.rx())));
            painter->drawText(dHint.topLeft() + QPointF(3.0, 25), "dy: " + QString().sprintf("%4.4f", fabs(value.ry())));
        }
    }
}

QPointF ChartView::nearestValue(const QPointF &mappedValue)
{
    // считается, что график имеет линейную ось x
    #define SER_VEC(i) ((QLineSeries*)(m_chart->series()[0]))->pointsVector()[i];
    unsigned cur_size =((QLineSeries*)(m_chart->series()[0]))->pointsVector().size();
    bool doubled = false;
    // не смотря на то, что серия была заполнена конкретным числом точек,
    // размер вектора все равно почему-то варьируется
    if(cur_size == m_serVecSize*2 - 1) doubled = true;
    QPointF begin = SER_VEC(0);
    QPointF next = SER_VEC(1);
    int delta = (mappedValue.x() - begin.x())/(next.x()-begin.x()),
        index = !doubled ? delta : ((delta > 0) ? delta*2-1 : 0);
    if(index >= cur_size) return SER_VEC(cur_size-1);
    return SER_VEC(index);
}

void ChartView::drawAxesLabels(QPainter *painter)
{
    QList<QString> axesLabels;
    axesLabels  << "ПЧ, мВ"   << "t, мкс"
                << "Ф, °"     << "t, мкс"
                << "F, МГц"   << "t, мкс"
                << "A, мВ∙мкс" << "f, МГц";
    QRect wid_area = this->rect();
    painter->drawText(QPointF(wid_area.left()+30, wid_area.top()+52), axesLabels[chartType*2]);
    painter->drawText(QPointF(wid_area.right()/2, wid_area.bottom()-20), axesLabels[chartType*2+1]);
}

void ChartView::calcHint(const QPointF &pos, QRectF &hint, const QRectF &area,
                         const unsigned coefX, const unsigned coefY)
{
    if(pos.x() - coefX*hintShift - coefX*hintWidth > area.left())
        hint.setLeft(pos.x() - coefX*hintShift - coefX*hintWidth);
    else hint.setLeft(pos.x() + coefX*hintShift);
    if(pos.y() - coefY*hintShift - coefY*hintHeight > area.top())
        hint.setTop(pos.y() - coefY*hintShift - coefY*hintHeight);
    else { // coefY-1 - потому что необдимо отступить вниз на 1 высоту меньше
        if(pos.y() - (coefY-1)*hintShift - (coefY-1)*hintHeight > area.top() && coefY > 1)
            hint.setTop(pos.y() + (coefY-1)*hintShift + (coefY-2)*hintHeight);
        else hint.setTop(pos.y() + coefY*hintShift + (coefY-1)*hintHeight);
    }
    hint.setWidth(hintWidth);
    hint.setHeight(hintHeight);
}

void ChartView::drawZoomRect(const QPointF prevPos, QPainter* painter)
{
    if(m_zoomRect == QRectF()) m_zoomRect = QRectF(prevPos, m_prevMousePos);
    else{
        if(m_mousePos.x() <  m_chart->plotArea().left())
            m_zoomRect.setRight(m_chart->plotArea().left());
        else if(m_mousePos.x() >  m_chart->plotArea().right())
            m_zoomRect.setRight(m_chart->plotArea().right());
        else m_zoomRect.setRight(m_mousePos.x());

        if(m_mousePos.y() <  m_chart->plotArea().top())
            m_zoomRect.setBottom(m_chart->plotArea().top());
        else if (m_mousePos.y() >  m_chart->plotArea().bottom())
            m_zoomRect.setBottom(m_chart->plotArea().bottom());
        else m_zoomRect.setBottom(m_mousePos.y());
    }
    if(!mouse_pressed){
        painter->setPen(QPen(QColor(122, 130, 144), hintBorderWidth));
        painter->drawRect(m_zoomRect);
    }
}
