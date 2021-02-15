#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <vector>
#include <QObject>
#include <QDebug>
#include <QCursor>
#include <QStack>
#include <QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

using namespace QtCharts;
#define ZoomCoef 2
#define ZoomMax 10
#define GRIDCOUNT 256

class ChartView : public QChartView
{
    Q_OBJECT
public:
    ChartView(QWidget *parent = nullptr);
    ChartView(QChart* chart, QWidget *parent = nullptr);
    void setAxisAndRange(QValueAxis *axisX, QValueAxis *axisY);
private:
    enum Axis{
      x, y
    };
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void drawForeground(QPainter* painter, const QRectF& rect);
    void resetAxis(QValueAxis* axis);
    void resetZoomedAxis(QValueAxis* axis);
    void getZoomedAxes();
    void drawCross(const QPointF point);
    QPointF nearestNode(float x, float y);
    QPoint m_mousePos;
    QChart* m_chart;
    QValueAxis* m_axisX, *m_axisY;
    QLineSeries* cross;
    int     oldx = 0, oldy = 0,
            frstx = 0, frsty = 0;
    // сетка, первые GRIDCOUNT значений - сетка икса
    // вторые GRIDCOUNT+1 - игрека
    // нужна для зацепления мышки за ближайший узел сетки
    double     grid[2*GRIDCOUNT + 1];
    // максимум ZoomMax зумов по иксу и игреку
    unsigned short zoomcountX = 0, zoomcountY = 0;
    const unsigned short hintWidth = 80, hintHeight = 30,
                         hintShift = 10, hintPadding = 1;
    double minX = 0, maxX = 0, minY = 0, maxY = 0,
           zoomedminX = 0, zoomedmaxX = 0,
           zoomedminY = 0, zoomedmaxY = 0;
    bool    chartmoving = false;
    bool    anchorX = false, anchorY = false,
            xmoving = false, ymoving = false;
signals:
    void arrowPressed(Qt::Key);
public slots:
};
#endif // CHARTVIEW_H
