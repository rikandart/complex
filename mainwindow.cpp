#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSysInfo>
#include <QHostInfo>

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
    graphTabInit();
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
    delete m_graphView;
    delete m_graphScene;
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
        qDebug() << this->height()-85;
        m_graphView->resize(this->width()-24, this->height()-85);
        frst = false;
    } else{
        QSize delta = event->size() - event->oldSize();
        m_graphView->resize(m_graphView->size()+delta);
    }

    /*ui->tabWidget->resize(ui->tabWidget->size() + delta);
    quint64 wCount = ui->tabWidget->count();
    for(double i = 0; i < wCount/2.0; i++){
        QWidget* widget1 = ui->tabWidget->widget(i*2);
        QWidget* widget2;
        if(i*2+1 < wCount)
            widget2 = ui->tabWidget->widget(i*2+1);
    }*/

}

void MainWindow::graphTabInit()
{
    ui->tab_2->setSizePolicy(ui->tabWidget->sizePolicy());
    m_graphView = new QGraphicsView(ui->tab_2);
    m_graphScene = new QGraphicsScene();
    m_graphView->setScene(m_graphScene);
//    m_graphView->resize(ui->tabWidget->width()-10, ui->tabWidget->height()-50);
    qDebug() << ui->tabWidget->size();
    m_graphScene->addLine(0, 150, 150, 0);
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
            DataProcessor data_pr;
            data_pr.Read(m_qfsm->filePath(index));
        }
    }

}
