#include "graphview.h"

GraphView::GraphView(QGraphicsView *view): mainView(view){}

GraphView::GraphView(QWidget *parent): QGraphicsView(parent){}

GraphView::~GraphView()
{
    if(mainView) delete mainView;
}

void GraphView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;
    double numdeg = (event->angleDelta() / 8).y()/7.5;
    qDebug() << numdeg;
    numdeg = (numdeg > 0) ? numdeg : 1/(-numdeg);
    emit scaleChanged(numdeg);
}

