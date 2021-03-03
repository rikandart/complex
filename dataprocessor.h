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
#include <QtCharts/QLegendMarker>
#include <QValueAxis>
#include <complex>
#include <vector>
#include <memory>
#include "cuza.h"
#define NCOUNT 100
#define SWEEP_WINDOWS 5
using namespace QtCharts;

typedef std::complex<double> complex;
typedef std::shared_ptr<complex> complex_ptr;
typedef std::shared_ptr<qint16> qint16_ptr;

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
    void oscOutput(QLineSeries** &series, QChart* chart = nullptr, bool prev = false);
    // расчет спектра
//    QVector<float> getSpectrum();

    unsigned getScale() const;
    void resizeCheck(const unsigned len, const unsigned width);
private:
    /*  win_offset - с какого окна начинать вывод отсчетов
     *  win_i - индекс этого окна   */
    unsigned win_i = 0, offset = 0;
    double scale = 1;
    QLineSeries** m_lineseries = nullptr;
    QChart* m_chart = nullptr;
    size_t bufLen = 0;
    // расчет прямого и обратного бпф и дпф
    // qint16_ptr - разделяемый указатель на отсчеты
    // spectrum - разделяемый указатель на спектр, N - размерность бпф или дпф
    complex_ptr dft(const std::vector<qint16>& in, const unsigned N);
    complex_ptr fft(const std::vector<qint16>& in, const unsigned N);
    complex_ptr inv_fft(const complex_ptr spectrum, const unsigned N);
    complex_ptr inv_dft(const complex_ptr spectrum, const unsigned N);
    // преобразование гильберта для извлечения огибающей
    complex_ptr hilbert(const std::vector<qint16>& in, const unsigned N);
    // извлечение огибающей
    void envelope(std::vector<qint16>& out, const unsigned start, const unsigned end);
public slots:
    void setScale (const double& value);
};

#endif // DATAPROCESSOR_H
