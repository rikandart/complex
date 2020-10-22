#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

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
    /*splitter = new QSplitter(this->centralWidget());
    splitter->setOrientation(Qt::Horizontal);
    // set position for splitter
    this->setCentralWidget(splitter);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,1);*/
#ifndef QT_DEBUG
    setPath(m_qfsm->myComputer().toString());
#else
    this->setPath("");
#endif
}

MainWindow::~MainWindow()
{
    //delete splitter;
    delete ui;
}

void MainWindow::setPath(const QString &path)
{
    m_qfsm->setRootPath(path);
    ui->fileTree->setRootIndex(m_qfsm->index(path));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSize delta = event->size() - event->oldSize();
    ui->tabWidget->resize(ui->tabWidget->size() + delta);
    quint64 wCount = ui->tabWidget->count();
    for(double i = 0; i < wCount/2.0; i++){
        // 091020 speed up the algorithm
        QWidget* widget1 = ui->tabWidget->widget(i*2);
        QWidget* widget2;
        if(i*2+1 < wCount)
            widget2 = ui->tabWidget->widget(i*2+1);
    }

}


void MainWindow::on_pathTo_textChanged(const QString &arg1)
{
    if(arg1.length()&&(arg1.lastIndexOf('/') == arg1.length()-1 ||
       arg1.lastIndexOf('\\') == arg1.length()-1))
        setPath(arg1);
}
