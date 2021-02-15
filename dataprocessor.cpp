#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){
}

DataProcessor::~DataProcessor(){}

void DataProcessor::Read(const QString &filename)
{
    // считать лучше весь файл
    // длина окна определяется по поиску следующего сигнала синхронизации
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream d_stream(&file);
    Cuza& cuza = Cuza::get();
    unsigned win_len = cuza.getSampWinLen();
    cuza.cleanMainBuffer();
    if(filename.compare(cuza.getFilename(), Qt::CaseSensitive)){
        unsigned f_len = (file.size() >= cuza.getBuffLength()) ?
                          cuza.getBuffLength() : file.size();
        cuza.setFilename(filename);
        cuza.setWinCount(f_len/(2.0*win_len));
    }
    d_stream.skipRawData(cuza.incWinIndex()*2*win_len);
    for(unsigned i = 0; i < 2*win_len; i++){
        uchar element;
        d_stream >> element;
        cuza.appendToBuffer(element);
    }
    cuza.retrieveSync();
    cuza.retriveWinTime();
    cuza.retrieveSamples();
}

void DataProcessor::oscOutput(QLineSeries **series, QChart* chart)
{
    Q_ASSERT(series != nullptr || m_lineseries != nullptr);
    Q_ASSERT(chart != nullptr || m_chart != nullptr);
    Cuza& cuza = Cuza::get();
    if(chart) m_chart = chart;
    if(series) m_lineseries = series;
    // ремув убирает все данные о выведенных точках на график, а также
    // почему-то удаляет все данные о точках из кучи
    m_chart->removeAllSeries();
    (*m_lineseries) = new QLineSeries;
    (*m_lineseries)->clear();
    unsigned sampling_rate = cuza.getFd();
    unsigned winlen = cuza.getSampWinLen(),
             winIndex = cuza.getWinIndex();
    double offset = (winIndex-1)*winlen;
    for(unsigned i = 0; i < winlen; i++){
        (*m_lineseries)->append((i+offset)*(1.0/sampling_rate), cuza.getSample(i));
    }
    m_chart->addSeries((*m_lineseries));
    m_chart->createDefaultAxes();
    m_chart->axisY()->setRange(-2048, 2048);
}

unsigned DataProcessor::getScale() const
{
    return scale;
}

void DataProcessor::resizeCheck(const unsigned len, const unsigned width)
{
    if(len*scale< width)
        scale = width/(double)(len);
}

void DataProcessor::setScale(const double& value)
{
    Q_ASSERT(value > 0);
    const unsigned couBuf = NCOUNT*bufLen;
}
