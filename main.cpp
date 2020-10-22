#include "mainwindow.h"
#include <fileExplorer.h>
#include <QSplashScreen>
#include <QPixmap>
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    /*Form form;
    form.show();*/
#ifndef QT_DEBUG
    QSplashScreen splash(QPixmap("C:/Users/Pizhun_VD/Documents/planirovka.jpg"));
    splash.show();
    QThread::sleep(5);
#endif
    return a.exec();
}
