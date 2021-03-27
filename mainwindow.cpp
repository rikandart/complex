#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSysInfo>
#include <QHostInfo>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_qfsm = new QFileSystemModel;
    m_qfsm->setNameFilters(QStringList("*.dat"));
    m_qfsm->setNameFilterDisables(false);
    ui->fileTree->setModel(m_qfsm);
    ui->fileTree->setSortingEnabled(false);
    m_stim = new QStandardItemModel(0, 4, this);
    QString headers[4] = {"Имя", "Размер", "Тип", "Дата изменения"};
    for(unsigned i = 0; i < 4; i++) m_stim->setHeaderData(i, Qt::Horizontal, headers[i]);
    ui->fileTree->header()->setModel(m_stim);
    ui->fileTree->header()->resizeSection(0, 445);
    m_dataPr = new DataProcessor;
    m_series = new QLineSeries*[Cuza::get().getSeriesCount()];
    m_charts = new QChart*[Cuza::get().getSeriesCount()];
    m_chViews = new ChartView*[Cuza::get().getSeriesCount()];
    graphTabInit();
    QTimer::singleShot(0, this, SLOT(appReady()));
    /*
    splitter->setOrientation();
    // set position for splitter
    this->setCentralWidget(splitter);*/
#ifndef QT_DEBUG
    setPath(m_qfsm->myComputer().toString());
#else
    if(!QHostInfo::localHostName().compare("NIO-72-W11", Qt::CaseInsensitive))
        this->setPath("E:/CUZADATA/2009-SIRIUS/2009/05/14");
    else
        this->setPath("D:/cuza/CUZADATA/2009-SIRIUS/2009/05/14");
#endif
}

MainWindow::~MainWindow()
{
    delete m_stim;
    delete m_qfsm;
    /*delete m_graphView;
    delete m_graphScene;*/
    for(unsigned i = 0; i < Cuza::get().getChartCount(); i++){
        m_charts[i]->removeAllSeries();
        delete m_charts[i];
    }
    delete[] m_series;
    delete[] m_charts;
    delete ui;
}

void MainWindow::setPath(const QString &path)
{
    m_qfsm->setRootPath(path);
    ui->fileTree->setRootIndex(m_qfsm->index(path));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    static bool frst = true;
    if(frst){
        m_splitter[0]->move(-3,0);
        m_splitter[0]->resize(this->width()-24, this->height()-85);
        // m_graphView->resize(this->width()-24, this->height()-85);
        frst = false;
    } else{
        QSize delta = ui->tabWidget->size() - oldSize;
        oldSize = ui->tabWidget->size();
        m_splitter[0]->resize(m_splitter[0]->size()+delta);
        emit resized();
        //m_graphView->resize(m_graphView->size()+delta);
        //if(!m_graphScene->items().isEmpty()) m_dataPr->dispOutput();
    }
}

void MainWindow::graphTabInit()
{
    m_splitter[0] = new QSplitter(Qt::Horizontal, ui->tab_2);
    m_splitter[1] = new QSplitter(Qt::Vertical);
    m_splitter[2] = new QSplitter(Qt::Vertical);
    for(unsigned i = 1; i < splitters_count; i++)
        m_splitter[0]->addWidget(m_splitter[i]);
    for(unsigned i = 0; i < Cuza::get().getChartCount(); i++){
        if(i == ChartType::chOSC || i == ChartType::chPHASE || i == ChartType::chFREQ){
            m_chViews[i] = new ChartView((m_charts[i] = new QChart), ChartType(i));
            m_splitter[1]->addWidget(m_chViews[i]);
        } else {
            m_chViews[i] = new ChartView((m_charts[i] = new QChart), ChartType(i));
            m_splitter[2]->addWidget(m_chViews[i]);
        }
        QObject::connect(m_chViews[i], &ChartView::arrowPressed,
                         this, &MainWindow::redrawOsc);
        QObject::connect(this, &MainWindow::resized,
                         m_chViews[i], &ChartView::mainwinResized);
        QObject::connect(m_dataPr, &DataProcessor::setPointsVecSize,
                         m_chViews[i], &ChartView::receivePointsVecSize);
    }
    // соединение сигналов и слотов для chOsc, chPhase и chFreq
    const quint8 connChartCount = 3;
    for(int i = 0, j = 0; i < connChartCount && j < connChartCount; j++){
        if(i == j) continue;
        qDebug() << "i" << i << "j" << j;
        QObject::connect(m_chViews[ChartType(i)], &ChartView::transmitEvent,
                         m_chViews[ChartType(j)], &ChartView::receiveEvent);
        if(j == connChartCount-1 && i != connChartCount-1){
            i++;
            j = -1;
        }
    }

/*
    QObject::connect(m_chViews[ChartType::chPHASE], &ChartView::transmitEvent,
                     m_chViews[ChartType::chOSC], &ChartView::receiveEvent);
    QObject::connect(m_chViews[ChartType::chOSC], &ChartView::transmitEvent,
                     m_chViews[ChartType::chFREQ], &ChartView::receiveEvent);
    QObject::connect(m_chViews[ChartType::chPHASE], &ChartView::transmitEvent,
                     m_chViews[ChartType::chFREQ], &ChartView::receiveEvent);
    QObject::connect(m_chViews[ChartType::chFREQ], &ChartView::transmitEvent,
                     m_chViews[ChartType::chPHASE], &ChartView::receiveEvent);
    QObject::connect(m_chViews[ChartType::chFREQ], &ChartView::transmitEvent,
                     m_chViews[ChartType::chOSC], &ChartView::receiveEvent);
                     */
    // graphview
    // is used for db
    /*m_graphView = new GraphView(ui->tab_3);
    m_graphScene = new QGraphicsScene();
    m_graphView->setScene(m_graphScene);
    m_graphView->setLineWidth(10);*/
    // QObject::connect(m_graphView, &GraphView::scaleChanged, m_dataPr, &DataProcessor::setScale);
    // m_graphView->resize(ui->tabWidget->width()-10, ui->tabWidget->height()-50);
}

void MainWindow::appReady()
{
    oldSize = ui->tabWidget->size();
}

void MainWindow::redrawOsc(Qt::Key key)
{
    auto redraw = [&](bool prev = false)->void{
        m_dataPr->oscOutput(m_series, m_charts, prev);
        for(unsigned i = 0; i < Cuza::get().getChartCount(); i++){
            m_chViews[i]->setUpdatesEnabled(true);
            m_chViews[i]->setAxisAndRange(static_cast<QValueAxis*>(m_charts[i]->axes()[0]),
                                         static_cast<QValueAxis*>(m_charts[i]->axes()[1]));
        }
    };
    switch(key){
        case Qt::Key_Right:
            redraw();
            m_dataPr->rightcount++;
        break;
        case Qt::Key_Left:
            redraw(true);
            m_dataPr->rightcount--;
        break;
    }
}


void MainWindow::on_pathTo_textChanged(const QString &arg1)
{
    if(arg1.length()&&(arg1.lastIndexOf('/') == arg1.length()-1 ||
       arg1.lastIndexOf('\\') == arg1.length()-1))
        setPath(arg1);
}

void MainWindow::on_fileTree_doubleClicked(const QModelIndex &index){
    if(m_qfsm->fileName(index).contains(".dat", Qt::CaseInsensitive)){
        if(INIProcessor().read(m_qfsm->filePath(index).
           replace(".dat", ".ini", Qt::CaseInsensitive))){
            m_dataPr->Read(m_qfsm->filePath(index));
            m_dataPr->oscOutput(m_series, m_charts);
            ui->tabWidget->setCurrentWidget(ui->tab_2);
            for(unsigned i = 0; i < Cuza::get().getChartCount(); i++)
                m_chViews[i]->setAxisAndRange(static_cast<QValueAxis*>(m_charts[i]->axisX()),
                                         static_cast<QValueAxis*>(m_charts[i]->axisY()));
            m_chViews[ChartType::chOSC]->setFocus();
        }
    }

}
