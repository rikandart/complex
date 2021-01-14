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
    // qDebug() << cuza;

}

void DataProcessor::dispOutput(QGraphicsScene *graphScene)
{
    Cuza& cuza = Cuza::get();
    bufLen = cuza.getNFFT();
    if(graphScene) m_graphScene = graphScene;
    const unsigned couBuf = NCOUNT*bufLen;
    unsigned viewWidth = m_graphScene->views().at(0)->width();
    resizeCheck(couBuf, viewWidth);
    m_graphScene->clear();
    for(unsigned i = 0; i < couBuf-1; i++){
        m_graphScene->addLine(i*scale, cuza.getBufValue(i), (i+1)*scale, cuza.getBufValue(i+1));
       /* if(i == bufLen/10){
            int p = 0;
            p++;
            break;
        }*/
    }
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
    unsigned viewWidth = m_graphScene->views().at(0)->width();
    if(couBuf*scale*value < viewWidth) return;
    scale *= value;
    dispOutput();
}
