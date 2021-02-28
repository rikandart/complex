#include "cuza.h"
#include <QThread>
#include <bitset>
#include <string>

Cuza Cuza::obj;

Cuza::Cuza(QObject *parent) : QObject(parent){}

Cuza::~Cuza()
{
    if(sampbuffer)  delete[] sampbuffer;
    if(mainbuffer)  delete[] mainbuffer;
}

unsigned short Cuza::getSeriesCount() const
{
    return m_series_count;
}

quint16 Cuza::getSync() const
{
    return sync;
}

void Cuza::retriveWinTime()
{
    winTime = 0;
    for(unsigned i = 16; i < 64; i++){
        winTime = (winTime << 1)|((mainbuffer[i*2+1] >> 7) & 1);
        if (i == 16)
            qDebug() << ((mainbuffer[i*2+1] >> 7) & 1);
    }
    qDebug() << "winTime" << winTime << QString::number(winTime, 2);
    // winTime /= fd;
    qDebug() << "winTime/fd" << winTime/fd << QString::number(winTime, 2);
}

void Cuza::cleanMainBuffer()
{
    resizeBuffer(mainBufferSize);
}

unsigned Cuza::getWinCount() const
{
    return winCount;
}

void Cuza::setWinCount(const unsigned &value)
{
    winCount = value;
}

uchar *Cuza::getMainbuffer() const
{
    return mainbuffer;
}

void Cuza::appendToBuffer(uchar value)
{
    Q_ASSERT(next_i < mainBufferSize);
    mainbuffer[next_i] = value;
    next_i++;
}

void Cuza::resizeBuffer(unsigned size)
{
    // удаляем старый буфер и создаем новый
    if (mainbuffer){
        delete[] mainbuffer;
        mainbuffer = NULL;
    }
    next_i = 0;
    mainbuffer = new uchar[size];
    mainBufferSize = size;
}

size_t Cuza::getBufferSize() const
{
    return mainBufferSize;
}

uchar Cuza::getBufValue(const unsigned i)
{
    Q_ASSERT(i < buffLength);
    return mainbuffer[i];
}

void Cuza::retrieveSamples()
{
    if(sampbuffer)  delete[] sampbuffer;
    sampbuffer = new qint16[mainBufferSize/2];
    qint16 sample_mask = static_cast<qint16>(pow(2, sampBitWidth)) - 1,
           inv_mask = ~sample_mask,
           sign_mask = static_cast<qint16>(pow(2, sampBitWidth-1));
    for(unsigned i = 0; i < mainBufferSize/2; i++){
        sampbuffer[i] = (((qint16*)mainbuffer)[i]) & sample_mask;
        if(sampbuffer[i] & sign_mask) sampbuffer[i] |= inv_mask;
    }
    delete[] mainbuffer;
    mainbuffer = NULL;
}

qint16 Cuza::getSample(const unsigned i)
{
    if(i >= mainBufferSize/2) qDebug() << "getSample error" << i;
    Q_ASSERT(i < mainBufferSize/2);
    return sampbuffer[i];
}

void Cuza::retrieveSync()
{
    // в 7м бите каждого второго байта первых 16ти слов
    // содержится бит синхроимпульса
    qDebug() << "-------------";
    sync = 0;
    qDebug() << "sync before" << QString::number(sync, 2) << QString::number(sync, 2).length();
    for(unsigned i = 0; i < 16; i++){
        sync = (sync << 1)|((mainbuffer[i*2+1] >> 7) & 1);
    }
    qDebug() << "sync after" << QString::number(sync, 2) << QString::number(sync, 2).length();
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
    sampWinLen /= 2;
    qDebug() << "sampWinLen in" << value << "sampWinLen out" << sampWinLen;
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
