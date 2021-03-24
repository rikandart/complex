#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H
#define _USE_MATH_DEFINES

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
#include <algorithm>
#include <fftw3.h>
#include "cuza.h"

#define NCOUNT 100
#define SWEEP_WINDOWS 5
#define ENV_COEF 1000
#define REAL 0
#define IMAG 1
#define ADC_COEF 2.048
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
    void oscOutput(QLineSeries** &series, QChart** charts = nullptr, bool prev = false);
    // расчет спектра
//    QVector<float> getSpectrum();

    unsigned getScale() const;
    void resizeCheck(const unsigned len, const unsigned width);
    unsigned rightcount = 0;
private:
    /*  win_offset - с какого окна начинать вывод отсчетов
     *  win_i - индекс этого окна   */
    unsigned win_i = 0, offset = 0;
    double scale = 1;
    QLineSeries** m_lineseries = nullptr;
    QChart** m_charts = nullptr;
    size_t bufLen = 0;
    unsigned fftN = 0;
    bool hilb = false;
    // расчет прямого и обратного бпф и дпф
    // qint16_ptr - разделяемый указатель на отсчеты
    // spectrum - разделяемый указатель на спектр, N - размерность бпф или дпф
    void dft(fftw_complex *in, fftw_complex *out, const unsigned N);
    void idft(fftw_complex *in, fftw_complex *out, const unsigned N);
    void fft(fftw_complex *in, fftw_complex *out, const unsigned N);
    void ifft(fftw_complex *in, fftw_complex *out, const unsigned N);
    // преобразование гильберта для извлечения огибающей
    void hilbert(fftw_complex* in,  fftw_complex* out, const unsigned N);
    // расчет бпф и комплексного сигнала
    void calc_fft_comp_sig(fftw_complex* fft_res, fftw_complex* complex_sig,
                           const unsigned start, const unsigned end, fftw_complex* input = nullptr);
signals:
    void setPointsVecSize(unsigned size, ChartType type);
public slots:
    void setScale (const double& value);
};

#endif // DATAPROCESSOR_H
