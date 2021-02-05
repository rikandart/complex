#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){
}

DataProcessor::~DataProcessor(){
}

void DataProcessor::Read(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream d_stream(&file);
    Cuza& cuza = Cuza::get();
    unsigned f_len = (file.size() >= cuza.getBuffLength()) ?
                      cuza.getBuffLength() : file.size();
    cuza.resizeBuffer(f_len);
    unsigned offset_step = f_len/(2*cuza.getMaxVisSamples());
    unsigned step = 0;
    for(unsigned i = 0; i < f_len+1; i++){
        uchar element;
        d_stream >> element;
        cuza.appendToBuffer(element);
    }
    cuza.retrieveSamples();
}

void DataProcessor::oscOutput(QLineSeries **series, QChart* chart)
{
    Q_ASSERT(series != nullptr || m_lineseries != nullptr);
    Q_ASSERT(chart != nullptr || m_chart != nullptr);
    Cuza& cuza = Cuza::get();
    if(chart) m_chart = chart;
    if(series) m_lineseries = series;
    /*unsigned viewWidth = m_lineseries->views().at(0)->width();
    resizeCheck(couBuf, viewWidth);*/
    // ремув убирает все данные о выведенных точках на график, а также
    // почему-то удаляет все данные о точках из кучи
    m_chart->removeAllSeries();
    (*m_lineseries) = new QLineSeries;
    (*m_lineseries)->clear();
    for(unsigned i = 0; i < cuza.getMaxVisSamples(); i++){
        (*m_lineseries)->append(i, cuza.getSample(i));
    }
    m_chart->addSeries((*m_lineseries));
    m_chart->createDefaultAxes();
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
    //unsigned viewWidth = m_lineseries->views().at(0)->width();
    //if(couBuf*scale*value < viewWidth) return;
    scale *= value;
    oscOutput();
}
