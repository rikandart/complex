#include "cuza.h"

Cuza Cuza::obj;

Cuza::Cuza(QObject *parent) : QObject(parent){}

int Cuza::getFreqInv() const{
    return freqInv;
}

void Cuza::setFreqInv(int value){
    freqInv = value;
}

float Cuza::getFd() const{
    return fd;
}

void Cuza::setFd(float value){
    fd = value;
}

float Cuza::getIfFreq() const{
    return ifFreq;
}

void Cuza::setIfFreq(float value){
    ifFreq = value;
}

float Cuza::getFreq() const{
    return freq;
}

void Cuza::setFreq(float value){
    freq = value;
}


Cuza& Cuza::get(){
    return obj;
}
