#include "iniprocessor.h"

INIProcessor::INIProcessor(QObject *parent) : QObject(parent){}

INIProcessor::~INIProcessor(){}

void INIProcessor::read(const QString& filename){
    if(filename.contains(".ini", Qt::CaseInsensitive)){
        QSettings ini(filename, QSettings::IniFormat);
        qDebug() << filename << ini.value("main/freq_inv").toInt() << ini.status();
    }
}
