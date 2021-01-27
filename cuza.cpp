#include "cuza.h"

Cuza Cuza::obj;

Cuza::Cuza(QObject *parent) : QObject(parent){}

Cuza::~Cuza()
{
    delete mainbuffer;
}

uchar *Cuza::getMainbuffer() const
{
    return mainbuffer;
}

void Cuza::appendToBuffer(uchar value)
{
    Q_ASSERT(next_i < buffLength);
    mainbuffer[next_i] = value;
    next_i++;
}

void Cuza::resizeBuffer(unsigned size)
{
    // удаляем старый буфер и создаем новый
    if (mainbuffer){
        delete mainbuffer;
        next_i = 0;
    }
    mainbuffer = new uchar[size];
    mainBufferSize = size;
    qDebug() << "Main buffer size" << mainBufferSize;
}

size_t Cuza::getBufferSize() const
{
    return mainBufferSize;
    // return sizeof (mainbuffer);
}

uchar Cuza::getBufValue(const unsigned i)
{
    Q_ASSERT(i < buffLength);
    return mainbuffer[i];
}

void Cuza::retrieveSamples()
{
    quint8 mask = 0b00001111;
    if(sampbuffer) delete sampbuffer;
    sampbuffer = new qint16[mainBufferSize/2];
    auto reversebits = [](quint16 number)->quint16{
        quint16 mask1_1 = 0b111111000000, mask1_2 = 0b000000111111,
                mask2_1 = 0b111000111000, mask2_2 = 0b000111000111,
                mask3_1 = 0b100100100100, mask3_2 = 0b001001001001, mask3_3 = 0b010010010010;
        number = (number & mask1_1) >> 6 | (number & mask1_2) << 6;
        number = (number & mask2_1) >> 3 | (number & mask2_2) << 3;
        number = (number & mask3_1) >> 2 | (number & mask3_2) << 2 | (number & mask3_3);
        return number;
    };
    for(unsigned i = 0; i < mainBufferSize/2; i++){
        quint16 tmp = (quint16(mask & mainbuffer[i*2]) << 4) + mainbuffer[i*2+1];
        //qDebug() << tmp << QString::number(tmp, 2);
        tmp = reversebits(tmp);
        //qDebug() << tmp << QString::number(tmp, 2);
        qint16 tmp2 = tmp;
        if((2048&tmp) >> 11){
            tmp2 = -(~(tmp-1)-61440);
            //qDebug() << "negative" << tmp2 << QString::number(tmp2, 2);
        }
        sampbuffer[i] = tmp2;
    }
}

qint16 Cuza::getSample(const unsigned i)
{
    Q_ASSERT(i < mainBufferSize/2);
    return sampbuffer[i];
}

Cuza::operator QString() const
{
   QString out = "";
   for(int i  = 0; i < mainBufferSize; i++){
       out += QString::number(mainbuffer[i]) + " ";
       if(i !=0 && i % 8 == 0.0)
           out += "\n";
   }
   return out;
}

bool Cuza::getCompMode() const
{
    return compMode;
}

void Cuza::setCompMode(bool value)
{
    compMode = value;
}

double Cuza::getFreqOffset() const
{
    return freqOffset;
}

void Cuza::setFreqOffset(double value)
{
    freqOffset = value;
}

unsigned Cuza::getFreqSMRatio() const
{
    return freqSMRatio;
}

void Cuza::setFreqSMRatio(const unsigned &value)
{
    freqSMRatio = value;
}

bool Cuza::getFreqSM() const
{
    return freqSM;
}

void Cuza::setFreqSM(bool value)
{
    freqSM = value;
}

unsigned Cuza::getSpectrogramPerekr() const
{
    return spectrogramPerekr;
}

void Cuza::setSpectrogramPerekr(const unsigned &value)
{
    spectrogramPerekr = value;
}

bool Cuza::getSoftLimiter() const
{
    return softLimiter;
}

void Cuza::setSoftLimiter(bool value)
{
    softLimiter = value;
}

bool Cuza::getFineMixer() const
{
    return fineMixer;
}

void Cuza::setFineMixer(bool value)
{
    fineMixer = value;
}

unsigned Cuza::getHidePoints() const
{
    return hidePoints;
}

void Cuza::setHidePoints(const unsigned &value)
{
    hidePoints = value;
}

double Cuza::getFreqShift() const
{
    return freqShift;
}

void Cuza::setFreqShift(double value)
{
    freqShift = value;
}

unsigned Cuza::getSpecrogramPoints() const
{
    return specrogramPoints;
}

void Cuza::setSpecrogramPoints(const unsigned &value)
{
    specrogramPoints = value;
}

double Cuza::getPhiCorr() const
{
    return phiCorr;
}

void Cuza::setPhiCorr(double value)
{
    phiCorr = value;
}

int Cuza::getFFTScale() const
{
    return FFTScale;
}

void Cuza::setFFTScale(int value)
{
    FFTScale = value;
}

double Cuza::getLinCorr() const
{
    return linCorr;
}

void Cuza::setLinCorr(double value)
{
    linCorr = value;
}

unsigned Cuza::getSpectrumPoints() const
{
    return spectrumPoints;
}

void Cuza::setSpectrumPoints(const unsigned &value)
{
    spectrumPoints = value;
}

SpectrumWindows Cuza::getSpectrumWindow() const
{
    return spectrumWindow;
}

void Cuza::setSpectrumWindow(const unsigned &value)
{
    spectrumWindow = SpectrumWindows(value);
}

SpectrumTypes Cuza::getSpectrumType() const
{
    return spectrumType;
}

void Cuza::setSpectrumType(const unsigned &value)
{
    spectrumType = SpectrumTypes(value);
}

bool Cuza::getShowFIPOnly() const
{
    return showFIPOnly;
}

void Cuza::setShowFIPOnly(bool value)
{
    showFIPOnly = value;
}

bool Cuza::getShowPIPOnly() const
{
    return showPIPOnly;
}

void Cuza::setShowPIPOnly(bool value)
{
    showPIPOnly = value;
}

bool Cuza::getShowCoarseVideo() const
{
    return showCoarseVideo;
}

void Cuza::setShowCoarseVideo(bool value)
{
    showCoarseVideo = value;
}

bool Cuza::getShowCoarseFreq() const
{
    return showCoarseFreq;
}

void Cuza::setShowCoarseFreq(bool value)
{
    showCoarseFreq = value;
}

bool Cuza::getShowSpectrum() const
{
    return showSpectrum;
}

void Cuza::setShowSpectrum(bool value)
{
    showSpectrum = value;
}

bool Cuza::getShowFreq() const
{
    return showFreq;
}

void Cuza::setShowFreq(bool value)
{
    showFreq = value;
}

bool Cuza::getShowPhase() const
{
    return showPhase;
}

void Cuza::setShowPhase(bool value)
{
    showPhase = value;
}

bool Cuza::getShowVideo() const
{
    return showVideo;
}

void Cuza::setShowVideo(bool value)
{
    showVideo = value;
}

bool Cuza::getShowRF() const
{
    return showRF;
}

void Cuza::setShowRF(bool value)
{
    showRF = value;
}

unsigned Cuza::getMaxTimeSamples() const
{
    return maxTimeSamples;
}

void Cuza::setMaxTimeSamples(const unsigned &value)
{
    maxTimeSamples = value;
}

unsigned Cuza::getMaxVisSamples() const
{
    return maxVisSamples;
}

void Cuza::setMaxVisSamples(const unsigned &value)
{
    maxVisSamples = value;
}

int64_t Cuza::getTimeStart() const
{
    return timeStart;
}

void Cuza::setTimeStart(const int64_t &value)
{
    timeStart = value;
}

int64_t Cuza::getSyncStart() const
{
    return syncStart;
}

void Cuza::setSyncStart(const int64_t &value)
{
    syncStart = value;
}

unsigned Cuza::getPorogFFT() const
{
    return porogFFT;
}

void Cuza::setPorogFFT(const unsigned &value)
{
    porogFFT = value;
}

double Cuza::getPorog() const
{
    return porog;
}

void Cuza::setPorog(double value)
{
    porog = value;
}

unsigned Cuza::getSampWinLen() const
{
    return sampWinLen;
}

void Cuza::setSampWinLen(const unsigned &value)
{
    sampWinLen = value;
}

unsigned Cuza::getDecimationRatio() const
{
    return decimationRatio;
}

void Cuza::setDecimationRatio(const unsigned &value)
{
    decimationRatio = value;
    sampWinLen = decimationRatio * (1 << NFFT);
}

unsigned Cuza::getSampBitWidth() const
{
    return sampBitWidth;
}

void Cuza::setSampBitWidth(const unsigned &value)
{
    sampBitWidth = value;
}

unsigned Cuza::getSampWidth() const
{
    return sampWidth;
}

void Cuza::setSampWidth(const unsigned &value)
{
    sampWidth = value;
}

unsigned Cuza::getNFFT() const
{
    return NFFT;
}

void Cuza::setNFFT(const unsigned &value)
{
    NFFT = value;
}

unsigned Cuza::getBuffLength() const
{
    return buffLength;
}

void Cuza::setBuffLength(const unsigned &value)
{
    buffLength = value;
}

unsigned Cuza::getMaxSamples() const
{
    return maxSamples;
}

void Cuza::setMaxSamples(const unsigned &value)
{
    maxSamples = value;
}

int Cuza::getVersion() const
{
    return version;
}

void Cuza::setVersion(int value)
{
    version = value;
}

unsigned Cuza::getCoeffCount() const
{
    return coeffCount;
}

void Cuza::setCoeffCount(const unsigned &value)
{
    coeffCount = value;
}

QList<QVector<unsigned> > Cuza::getCoefNums() const
{
    return coefNums;
}

void Cuza::setCoefNums(const QList<QVector<unsigned> > &value)
{
    coefNums = value;
}

QVector<unsigned> Cuza::getCoeffs() const
{
    return coeffs;
}

void Cuza::setCoeffs(const QVector<unsigned> &value)
{
    coeffs = value;
}

unsigned Cuza::getCoeffLen() const
{
    return coeffLen;
}

void Cuza::setCoeffLen(const unsigned &value)
{
    coeffLen = value;
}

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
