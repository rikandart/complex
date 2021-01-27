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
    unsigned f_len = (file.size() >= cuza.getBuffLength()) ? cuza.getBuffLength() : file.size();
    qDebug() << f_len << file.size() << cuza.getBuffLength();
    cuza.resizeBuffer(f_len);
    for(unsigned i = 0; i < f_len+1; i++){
        uchar element;
        d_stream >> element;
        cuza.appendToBuffer(element);
    }
    cuza.retrieveSamples();
}

void DataProcessor::dispOutput(QLineSeries **series, QChart* chart)
{
    Cuza& cuza = Cuza::get();
    bufLen = cuza.getNFFT();
    if(chart) m_chart = chart;
    if(series) m_lineseries = series;
    const unsigned couBuf = NCOUNT*bufLen;
    /*unsigned viewWidth = m_lineseries->views().at(0)->width();
    resizeCheck(couBuf, viewWidth);*/
    m_chart->removeAllSeries();
    (*m_lineseries) = new QLineSeries;
    (*m_lineseries)->clear();
    for(unsigned i = 0; i < 65536/*couBuf-1*/; i++){
        // 220121
        // переделать на qtcharts
        /*qDebug() << cuza.getSample(i);*/
        (*m_lineseries)->append(i, cuza.getSample(i));
        // m_graphScene->addLine(i*scale, cuza.getSample(i)/20, (i+1)*scale, cuza.getSample(i)/20);
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
    dispOutput();
}
