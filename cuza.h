#ifndef CUZA_H
#define CUZA_H

#include <QObject>

// ???????? ??? ?????? ?????? ?? ini ?????
class Cuza : public QObject
{
    Q_OBJECT
public:
    static Cuza& get();
    float getFreq() const;
    void setFreq(float value);
    float getIfFreq() const;
    void setIfFreq(float value);
    float getFd() const;
    void setFd(float value);
    int getFreqInv() const;
    void setFreqInv(int value);

private:
    explicit Cuza(QObject *parent = nullptr);
    static Cuza obj;
    float freq = 0.0, ifFreq = 0.0, fd = 0.0;
    int freqInv = 0;
signals:

};

#endif // CUZA_H
