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
    m_dataPr = new DataProcessor;
    graphTabInit();
    QTimer::singleShot(0, this, SLOT(appReady()));
    /*splitter = new QSplitter(this->centralWidget());
    splitter->setOrientation(Qt::Horizontal);
    // set position for splitter
    this->setCentralWidget(splitter);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,1);*/
#ifndef QT_DEBUG
    setPath(m_qfsm->myComputer().toString());
#else
    if(!QHostInfo::localHostName().compare("NIO-72-W11", Qt::CaseInsensitive))
        this->setPath("E:/CUZADATA/2009-SIRIUS/2014/04/15");
    else
        this->setPath("D:/cuza/CUZADATA/2009-SIRIUS/2014/04/15");
#endif
}

MainWindow::~MainWindow()
{
    //delete splitter;
    delete m_qfsm;
    /*delete m_graphView;
    delete m_graphScene;*/
    m_chart->removeAllSeries();
    delete m_chart;
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
        m_chartView->resize(this->width()-24, this->height()-85);
        // m_graphView->resize(this->width()-24, this->height()-85);
        frst = false;
    } else{
        QSize delta = ui->tabWidget->size() - oldSize;
        oldSize = ui->tabWidget->size();
        m_chartView->resize(m_chartView->size()+delta);
        //m_graphView->resize(m_graphView->size()+delta);
        //if(!m_graphScene->items().isEmpty()) m_dataPr->dispOutput();
    }
}

void MainWindow::graphTabInit()
{
    ui->tab_2->setSizePolicy(ui->tabWidget->sizePolicy());
    m_chart = new QChart;
    m_chartView = new ChartView(m_chart, ui->tab_2);
    ui->tab_2->grabGesture(Qt::PanGesture);
    ui->tab_2->grabGesture(Qt::PinchGesture);
    m_series = new QLineSeries();
    m_chart->addSeries(m_series);
    m_chart->createDefaultAxes();
    m_chart->removeAllSeries();
    m_series = new QLineSeries();
    for(int i = -100; i < 100; i++) m_series->append(i, sqrt(i+100));
    m_chart->addSeries(m_series);
    m_chart->createDefaultAxes();
    m_chart->setTitle("Осциллограмма");
    m_chart->legend()->markers()[0]->setLabel("Сигнал");
    // graphview
    // is used for db
    /*m_graphView = new GraphView(ui->tab_3);
    m_graphScene = new QGraphicsScene();
    m_graphView->setScene(m_graphScene);
    m_graphView->setLineWidth(10);*/
    // QObject::connect(m_graphView, &GraphView::scaleChanged, m_dataPr, &DataProcessor::setScale);
    // m_graphView->resize(ui->tabWidget->width()-10, ui->tabWidget->height()-50);
    // qDebug() << ui->tabWidget->size();
}

void MainWindow::appReady()
{
    oldSize = ui->tabWidget->size();
}


void MainWindow::on_pathTo_textChanged(const QString &arg1)
{
    if(arg1.length()&&(arg1.lastIndexOf('/') == arg1.length()-1 ||
       arg1.lastIndexOf('\\') == arg1.length()-1))
        setPath(arg1);
}

void MainWindow::on_fileTree_doubleClicked(const QModelIndex &index){
    if(m_qfsm->fileName(index).contains(".dat", Qt::CaseInsensitive)){
        INIProcessor ini;
        if(ini.read(m_qfsm->filePath(index).
           replace(".dat", ".ini", Qt::CaseInsensitive))){
            m_dataPr->Read(m_qfsm->filePath(index));
            m_dataPr->dispOutput(&m_series, m_chart);
        }
    }

}
