#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QObject>
#include <QWidget>
#include <QDebug>

class GraphView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphView(QGraphicsView* view = nullptr);
    GraphView(QWidget* parent);
    ~GraphView();
    virtual void wheelEvent(QWheelEvent *event);
private:
    QGraphicsView* mainView;
signals:
    unsigned scaleChanged(double scale);

};

#endif // GRAPHVIEW_H
