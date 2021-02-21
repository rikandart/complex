#ifndef CUZA_H
#define CUZA_H

// этот класс хранит сигнал и его параметры

#include <QObject>
#include <QVector>
#include <QDebug>
#include <cmath>
#include <math.h>
#define THRESHOLD 100

// типы
// Тип выводимого на график спектра.
enum SpectrumTypes {
  stPower = 0, // Спектр мощности.
  stPhase,     // Фазовый спектр.
  stSin,       // Синусная составляющая амплитудного спектра.
  stCos        // Косинусная составляющая амплитудного спектра.
};

// Тип оконной функции.
enum SpectrumWindows {
  swNone = 0,     // Без окна.
  swParzen,       // Окно Парзена.
  swHanning,      // Окно Ханнинга.
  swWelch,        // Окно Уолша.
  swHamming,      // Окно Хамминга.
  swExactBlackman // Окно Блэкмена.
};

// синглтон для хранения мета данных из ini файла
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

    unsigned getCoeffLen() const;
    void setCoeffLen(const unsigned &value);

    QVector<unsigned> getCoeffs() const;
    void setCoeffs(const QVector<unsigned> &value);

    QList<QVector<unsigned> > getCoefNums() const;
    void setCoefNums(const QList<QVector<unsigned> > &value);

    unsigned getCoeffCount() const;
    void setCoeffCount(const unsigned &value);

    int getVersion() const;
    void setVersion(int value);

    unsigned getMaxSamples() const;
    void setMaxSamples(const unsigned &value);

    unsigned getBuffLength() const;
    void setBuffLength(const unsigned &value);

    unsigned getNFFT() const;
    void setNFFT(const unsigned &value);

    unsigned getSampWidth() const;
    void setSampWidth(const unsigned &value);

    unsigned getSampBitWidth() const;
    void setSampBitWidth(const unsigned &value);

    unsigned getDecimationRatio() const;
    void setDecimationRatio(const unsigned &value);

    unsigned getSampWinLen() const;
    // устанавливает длину окна в отсчетах и изменяет размер буфера
    void setSampWinLen(const unsigned &value);

    double getPorog() const;
    void setPorog(double value);

    unsigned getPorogFFT() const;
    void setPorogFFT(const unsigned &value);

    int64_t getSyncStart() const;
    void setSyncStart(const int64_t &value);

    int64_t getTimeStart() const;
    void setTimeStart(const int64_t &value);

    unsigned getMaxVisSamples() const;
    void setMaxVisSamples(const unsigned &value);

    unsigned getMaxTimeSamples() const;
    void setMaxTimeSamples(const unsigned &value);

    bool getShowRF() const;
    void setShowRF(bool value);

    bool getShowVideo() const;
    void setShowVideo(bool value);

    bool getShowPhase() const;
    void setShowPhase(bool value);

    bool getShowFreq() const;
    void setShowFreq(bool value);

    bool getShowSpectrum() const;
    void setShowSpectrum(bool value);

    bool getShowCoarseFreq() const;
    void setShowCoarseFreq(bool value);

    bool getShowCoarseVideo() const;
    void setShowCoarseVideo(bool value);

    bool getShowPIPOnly() const;
    void setShowPIPOnly(bool value);

    bool getShowFIPOnly() const;
    void setShowFIPOnly(bool value);

    SpectrumTypes getSpectrumType() const;
    void setSpectrumType(const unsigned &value);

    SpectrumWindows getSpectrumWindow() const;
    void setSpectrumWindow(const unsigned &value);

    unsigned getSpectrumPoints() const;
    void setSpectrumPoints(const unsigned &value);

    double getLinCorr() const;
    void setLinCorr(double value);

    int getFFTScale() const;
    void setFFTScale(int value);

    double getPhiCorr() const;
    void setPhiCorr(double value);

    unsigned getSpecrogramPoints() const;
    void setSpecrogramPoints(const unsigned &value);

    double getFreqShift() const;
    void setFreqShift(double value);

    unsigned getHidePoints() const;
    void setHidePoints(const unsigned &value);

    bool getFineMixer() const;
    void setFineMixer(bool value);

    bool getSoftLimiter() const;
    void setSoftLimiter(bool value);

    unsigned getSpectrogramPerekr() const;
    void setSpectrogramPerekr(const unsigned &value);

    bool getFreqSM() const;
    void setFreqSM(bool value);

    unsigned getFreqSMRatio() const;
    void setFreqSMRatio(const unsigned &value);

    double getFreqOffset() const;
    void setFreqOffset(double value);

    bool getCompMode() const;
    void setCompMode(bool value);

    // перед добавлением элементов в буфер
    // необходимо изменить его размер
    uchar *getMainbuffer() const;
    void appendToBuffer(uchar value);
    void resizeBuffer(unsigned size);
    size_t getBufferSize() const;
    uchar getBufValue(const unsigned i);
    void retrieveSamples();
    qint16 getSample(const unsigned i);
    void retrieveSync();
    quint16 getSync() const;
    void retriveWinTime();

    operator QString() const;
    unsigned getWinCount() const;
    void setWinCount(const unsigned &value);

    void cleanMainBuffer();

private:
    explicit Cuza(QObject *parent = nullptr);
    ~Cuza();
    static Cuza obj;                // объект синглтона

    float       freq = 0.0,         // полная входная частота в полосе 0.3-18 ГГц
                ifFreq = 0.0,       // промежуточная частота (262.5)
                fd = 0.0;           // частота дискретизации (350 МГц)

    int         freqInv = 0,        // инверсия частоты на основе зоны Найквиста
                version = 0,        // версия
                FFTScale = 0;       // тип шкалы по амплитуде в спектре

    unsigned    coeffCount = 0,     // характеристики коэффициентов
                coeffLen = 0,
                maxSamples = 0,     // характеристики буфера отсчетов
                buffLength = 0,
                NFFT = 0,
                sampWidth = 0,      // размер семпла в байтах
                sampBitWidth = 0,   // размер семпла в битах
                decimationRatio = 0,// величина децимации
                sampWinLen = 0,     // размер окна в отсчетах
                porogFFT = 0,       // порог по БПФ
                maxVisSamples = 0,  // максимальное число отсчетов
                                    // для вывода на экран
                spectrumPoints = 0, // количество точек FFT
                specrogramPoints = 0,// количество точек спектрограммы
                hidePoints = 0,     // сколько скрывать точек перед выводом на график
                spectrogramPerekr = 0,// количество точек перекрытия гистограммы
                freqSMRatio = 0,    // коэффициент усреднения (2,4,6,8 ...)
                next_i = 0,         // индекс следующего элемента для записи в буфер
                winCount = 0;       // количество окон

    quint16     sync = 0;           // номер отсчета синхроимпульса
    quint64     winTime = 0;        // время окна в отсчетах


    double      porog = 0.0,        // порог для измерений
                linCorr = 0.0,      // cмещение DC
                phiCorr = 0.0,      // фазовый сдвиг в режиме сравнения
                freqShift = 0.0,    // смещение по частоте в режиме сравнения
                amplRatio = 0.0,    // масштабирующий коэффициент усиления в режиме сравнения
                freqOffset = 0.0;   // cмещение по частоте для переноса на среднюю частоту

    int64_t     syncStart = 0,      // Время реального прихода синхроимпульса
                                    // от запуска таймера
                timeStart = 0,      // Время в тактах частоты дискретизации
                                    // запуска таймера (1 ms точность)
                maxTimeSamples = 0; // максимальное число временных
                                    // отсчетов в буфере
    bool        showRF = 0,         // показывать отсчеты АЦП
                showVideo = 0,      // показывать огибающую
                showPhase = 0,      // показывать фазу
                showPIPOnly = 0,    // показывать фазу в пределах импульса
                showFreq = 0,       // показывать частоту
                showFIPOnly = 0,    // показывать частоту в пределах импульса
                showSpectrum = 0,   // показывать спектр
                showCoarseFreq = 0, // показывать грубую частоту
                showCoarseVideo = 0,// показывать грубую огибающую
                fineMixer = 0,      // cносить на среднюю частоту
                softLimiter = 0,    // программное ограничение огибающей
                freqSM = 0,         // усреднение частоты
                compMode = 0;       // режим сравнения

    size_t      mainBufferSize = 0;
    SpectrumTypes   spectrumType;   // тип выводимого спектра
    SpectrumWindows spectrumWindow; // тип оконной функции для расчета спектра

    QVector<unsigned> coeffs;
    QList<QVector<unsigned>> coefNums;

    uchar*       mainbuffer;
    qint16*      sampbuffer;

signals:

};

#endif // CUZA_H
