#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QDir>
#include <QSplitter>
#include <QResizeEvent>
#include <QGraphicsView>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLegendMarker>
#include <QValueAxis>
#include <QGraphicsScene>
#include <QPointF>
#include "chartview.h"
#include "graphview.h"
#include "dataprocessor.h"
#include "iniprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace QtCharts;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pathTo_textChanged(const QString &arg1);

    void on_fileTree_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    QFileSystemModel *m_qfsm;
    QStandardItemModel *m_stim;
    GraphView * m_graphView;
    QGraphicsScene * m_graphScene;
    // виджеты графиков осциллограммы и огибающей, фазы и спектра
    ChartView** m_chViews;
    QChart** m_charts;
    QLineSeries** m_series;
    QSplitter* splitter;
    DataProcessor* m_dataPr;
    QSize oldSize;
    void setPath(const QString& path);
    virtual void resizeEvent(QResizeEvent* event) override;
    void graphTabInit();
public slots:
    // слот вызываемый по готовности приложения
    void appReady();
    void redrawOsc(Qt::Key);
signals:
    void rebuildGrid();
};

class FileSystemModel: public QFileSystemModel{
private:
    virtual void setHeaderData();
};

#endif // MAINWINDOW_H
