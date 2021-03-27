#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <math.h>
#include <QObject>
#include <QDebug>
#include <QCursor>
#include <QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegendMarker>
#include <cuza.h>
#include <QTimer>

using namespace QtCharts;
#define ZoomCoef 2.0
#define ZoomMax (1/ZoomCoef)*10
#define GRIDCOUNT 256

class ChartView : public QChartView
{
    Q_OBJECT
public:
    ChartView(QWidget *parent = nullptr);
    ChartView(QChart* chart, ChartType type, QWidget *parent = nullptr);
    void setAxisAndRange(QValueAxis *axisX, QValueAxis *axisY);
private:
    enum Axis{
      x, y
    };

    enum InputEvent{
      MWHEEL, MPRESS, MDOUBLECLICK, MRELEASE, MMOVE,
      KPRESS, KRELEASE
    };
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void drawForeground(QPainter* painter, const QRectF& rect);
    // рисует подсказку с оординатами
    void drawHint(QPainter* painter, QPointF pos, bool label = false);
    // расчитывает расположение метки на графике относительно area (площадь графика)
    // заполняет прямоугольники hint, hintBorder
    // coefX, coefY - коэффициенты увеличения метки для расстояния от pos
    void calcHint(const QPointF& pos, QRectF& hint, const QRectF& area,
                  const unsigned coefX = 1, const unsigned coefY = 1);
    // функция зума
    void zoomChart(QPointF pos, int ang_data, QRectF zoomArea = QRectF());
    void zoomChart(QRectF zoomArea, bool zoomIn = true);
    // рисует прямоугольник для увеличения
    void drawZoomRect(const QPointF prevPos, QPainter* painter);
    // рисует названия осей
    void drawAxesLabels(QPainter* painter);
    // сбрасывает ось до начального состояния
    void resetAxis(QValueAxis* axis);
    // сбрасывает ось в зумированое состояние
    void resetZoomedAxis(QValueAxis* axis);
    // запомнить зумированное состояние
    void getZoomedAxes();
    // расчет сетки узлов для приближения
    void buildGrid();
    // центрирование графика по вертикали
    void centerVertically();
    // ищет ближайший угол сетки
    QPointF nearestNode(float x, float y);
    // ищетближайшее значение для отображения подсказки
    QPointF nearestValue(const QPointF& mappedValue);
    // m_prevMousePos - значение положения мыши в точке нажатия правой кнопки мыши
    // m_mousePos - текущее положение мыши
    QPointF m_prevMousePos, m_mousePos;
    // график
    QChart* m_chart = nullptr;
    // оси графика
    QValueAxis* m_axisX = nullptr, *m_axisY = nullptr;
    // предыдущий размер виджета
    QSize oldSize;
    // прямоугольник зумирования
    QRectF m_zoomRect;
    // тип графика
    ChartType chartType;
    // кол-во меток на графике, время обновления вида виджета
    static const uchar lab_count = 2, timer_msec = 50;
    // массив с расположениями метокна графике
    QList<QPointF> labels;
    int     oldx = 0, oldy = 0;
    unsigned m_serVecSize = 0;
    // сетка, первые GRIDCOUNT значений - сетка икса
    // вторые GRIDCOUNT+1 - игрека
    // нужна для зацепления мышки за ближайший узел сетки
    double     grid[2*GRIDCOUNT + 1];
    // максимум ZoomMax зумов по иксу и игреку
    double zoomcountX = 0, zoomcountY = 0;
    // размеры прямоугольника с подсказкой
    const unsigned short hintWidth = 80, hintHeight = 30,
                         hintShift = 10, hintBorderWidth = 1;
    // крайние точки осей
    double  minX = 0, maxX = 0, minY = 0, maxY = 0,
            zoomedminX = 0, zoomedmaxX = 0, // зумированные крайние точки осей
            zoomedminY = 0, zoomedmaxY = 0;
    bool    chartmoving = false;
    // anchorX - нажат ctrl, anchorY - нажат shift
    // при нажатии shift перемещать график можно относительо одной из осей
    bool    anchorX = false, anchorY = false,
            xmoving = false, ymoving = false;
    // поставить метку
    bool    set_label = false;
    // используется для согласованности осциллограммы и графика фазы
    // receiving - флаг прием qt-сигнала через receiveEvent
    bool    receiving = false;
    // флаг нажатия кнопки мыши
    bool    mouse_pressed = false;
signals:
    void arrowPressed(Qt::Key);
    // передача какого либо события другому такому же виджету
    void transmitEvent(InputEvent type, QInputEvent* event);
public slots:
    // размеры окна изменены
    void mainwinResized();
    // прием события от другого виджета
    void receiveEvent(InputEvent type, QInputEvent* event);
    // размер вектора точек с данными
    void receivePointsVecSize(unsigned size, ChartType type);
    // перерисовка внешнего вида виджета
    void updateView();
};
#endif // CHARTVIEW_H
