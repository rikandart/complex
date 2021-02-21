#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QVector>
#include <QStack>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QValueAxis>
#include "cuza.h"
#define NCOUNT 100
#define SWEEP_WINDOWS 5
using namespace QtCharts;

namespace Ui {
class FileExplorer;
}

class DataProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DataProcessor(QObject *parent = nullptr);
    ~DataProcessor();
    // чтение файла
    void Read(const QString& filename);
    // рисует график отсчетов из файла
    // запоминает указатели на серию точек и самого графика
    // вызов без аргументов перерисовывает график
    // флаг prev для прорисовки предыдущего сигнала
    void oscOutput(QLineSeries** series = nullptr, QChart* chart = nullptr, bool prev = false);
    // расчет спектра
//    QVector<float> getSpectrum();

    unsigned getScale() const;
    void resizeCheck(const unsigned len, const unsigned width);
private:
    /*  win_offset - с какого окна начинать вывод отсчетов
     *  win_i - индекс этого окна   */
    unsigned win_i = 0, win_offset = 0;
    double scale = 1;
    QLineSeries** m_lineseries = nullptr;
    QChart* m_chart = nullptr;
    size_t bufLen = 0;
public slots:
    void setScale (const double& value);
};

#endif // DATAPROCESSOR_H
