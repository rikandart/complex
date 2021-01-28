#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QDebug>
#include <QtCharts/QChartView>
#include <QGestureEvent>
#include <QObject>

using namespace QtCharts;

class ChartView : public QChartView
{
    Q_OBJECT
public:
    ChartView(QWidget *parent = nullptr);
    ChartView(QChart* chart, QWidget *parent = nullptr);
private:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    QChart* m_chart;
    int oldx = 0, oldy = 0;
    bool chartmoving = false;
};
#endif // CHARTVIEW_H
