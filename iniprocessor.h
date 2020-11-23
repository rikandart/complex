#ifndef INIPROCESSOR_H
#define INIPROCESSOR_H

#include <QObject>
#include <QDebug>
#include <QSettings>

class INIProcessor : public QObject
{
    Q_OBJECT
public:
    explicit INIProcessor(QObject *parent = nullptr);
    ~INIProcessor();
    void read(const QString& filename);

signals:

private:
};

#endif // INIPROCESSOR_H
