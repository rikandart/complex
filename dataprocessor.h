#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QVector>
#include "cuza.h"

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
    // расчет спектра
//    QVector<float> getSpectrum();
    // выдача данных
    // smth getData();

private:
};

#endif // DATAPROCESSOR_H
