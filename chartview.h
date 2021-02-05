#ifndef CHARTVIEW_H
#define CHARTVIEW_H

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

class ChartView : public QChartView
{
    Q_OBJECT
public:
    ChartView(QWidget *parent = nullptr);
    ChartView(QChart* chart, QWidget *parent = nullptr);
    void resetZoom();
    void setAxisAndRange(QValueAxis *axisX, QValueAxis *axisY);
private:
    enum Zoom{
      x, y
    };
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    QChart* m_chart;
    QValueAxis* m_axisX, *m_axisY;
    int     oldx = 0, oldy = 0, dXsum = 0, dYsum = 0;
    // максимум ZoomMax зумов по иксу и игреку
    unsigned short zoomcountX = 0, zoomcountY = 0;
    unsigned minX = 0, maxX = 0, minY = 0, maxY = 0;
    bool    chartmoving = false;
    bool    zoomX = false, zoomY = false;
signals:
    void doubleClicked();
public slots:
};
#endif // CHARTVIEW_H
