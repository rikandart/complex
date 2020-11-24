#ifndef INIPROCESSOR_H
#define INIPROCESSOR_H

#include "cuza.h"

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QSet>
#include <QFileInfo>
#include "error.h"

class INIProcessor : public QObject
{
    Q_OBJECT
public:
    explicit INIProcessor(QObject *parent = nullptr);
    ~INIProcessor();
    void read(const QString& filename);

signals:

private:
    Cuza& cuza = Cuza::get();
    bool validate(const QSettings& ini);
    void error(const QString& msg);
    void warning(const QString& msg);
};

#endif // INIPROCESSOR_H
