#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QVector>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include "cuza.h"
#define NCOUNT 100
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
    void dispOutput(QLineSeries** series = nullptr, QChart* chart = nullptr);
    // расчет спектра
//    QVector<float> getSpectrum();
    // выдача данных
    // smth getData();

    unsigned getScale() const;
    void resizeCheck(const unsigned len, const unsigned width);
private:
    double scale = 1;
    QLineSeries** m_lineseries;
    QChart* m_chart;
    size_t bufLen = 0;
public slots:
    void setScale (const double& value);
};

#endif // DATAPROCESSOR_H
