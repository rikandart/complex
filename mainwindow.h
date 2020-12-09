#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <QSplitter>
#include <QResizeEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include "dataprocessor.h"
#include "iniprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    QGraphicsView * m_graphView;
    QGraphicsScene * m_graphScene;
    QSplitter* splitter;
    void setPath(const QString& path);
    virtual void resizeEvent(QResizeEvent* event) override;
    void graphTabInit();
};
#endif // MAINWINDOW_H
