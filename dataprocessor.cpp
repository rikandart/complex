#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){}

DataProcessor::~DataProcessor(){}

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
    qDebug() << cuza;

}
