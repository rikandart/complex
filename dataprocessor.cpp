#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){
}

DataProcessor::~DataProcessor(){}

void DataProcessor::Read(const QString &filename)
{
    win_i = offset = 0;
    // считать лучше весь файл
    // длина окна определяется по поиску следующего сигнала синхронизации
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream d_stream(&file);
    Cuza& cuza = Cuza::get();
    unsigned win_len = cuza.getSampWinLen();
    unsigned f_len = (file.size() >= cuza.getBuffLength()) ?
                      cuza.getBuffLength() : file.size();
    cuza.setWinCount(f_len/(2.0*win_len));
    cuza.resizeBuffer(f_len);
    for(unsigned i = 0; i < f_len; i++){
        uchar element;
        d_stream >> element;
        cuza.appendToBuffer(element);
    }
//    cuza.retrieveSync();
//    cuza.retriveWinLen();
    cuza.retrieveSamples();
}

void DataProcessor::oscOutput(QLineSeries** &series, QChart* chart, bool prev)
{
    Q_ASSERT(series != 0 || m_lineseries != nullptr);
    Q_ASSERT(chart != nullptr || m_chart != nullptr);
    Cuza& cuza = Cuza::get();
    if(chart) m_chart = chart;
    if(series) m_lineseries = series;
    // ремув убирает все данные о выведенных точках на график, а также
    // почему-то удаляет все данные о точках из кучи
    m_chart->removeAllSeries();
    enum Chart {
      SAMPS,    // отсчеты
      ENV,      // огибающая
    };
    m_lineseries = new QLineSeries*[Cuza::get().getSeriesCount()];
    for(unsigned short i = 0; i < cuza.getSeriesCount(); i++){
        m_lineseries[i] = new QLineSeries;
        m_lineseries[i]->clear();
    }

    // поиск сигнала по порогу
    const unsigned  sampling_rate = cuza.getFd(),
                    maxsamp = cuza.getMaxVisSamples(),
                    bufsize = cuza.getBufferSize();
    static unsigned prev_offset = offset;
    static bool next = false;
    // назначение сдвига в зависимости от направления движения по записи
    if(prev){
        if(!next) // если был предыдущий сигнал
            prev_offset = offset ? offset : bufsize/2-1;
        else offset = prev_offset ? prev_offset : bufsize/2-1;
        next = false;
    } else {
        if(!next) // если был предыдущий сигнал
            offset = (prev_offset >= bufsize/2-1) ? 0 : prev_offset;
        else {
            offset = (offset >= bufsize/2-1) ? 0 : offset;
            prev_offset = offset;
        }
        next = true;
    }
    const unsigned short max_noise_samples = 10,
                         max_end_noise_samples = 10*max_noise_samples;
#define MAX_SAMP
#ifndef MAX_SAMP
    for(unsigned i = 0; i < cuza.getMaxVisSamples(); i++){
            (*m_lineseries)->append(i, cuza.getSample(i));
    }
#else
    // поиск по всем отсчетам окна
    // если есть флаг prev, то поиск проходит в другую сторону
    for(bool start = 0;
        !start && (!prev ? offset < bufsize/2 : offset > 0);
        !prev ? offset++ : offset--){
        if(abs(cuza.getSample(offset)) > THRESHOLD){
            start = 1;
            // чтобы начиналось не сразу с сигнала
            if(offset != 0) {
                for(unsigned thrsh_cou = 0; thrsh_cou < max_end_noise_samples && (!prev ? offset > 0 : offset < bufsize/2-1);
                    !prev ? offset-- : offset++){
                    if(abs(cuza.getSample(offset)) > THRESHOLD) thrsh_cou = 0;
                    else thrsh_cou++;
                }
            }
        }
    }
    // если не был достигнут конец буфера
    if(offset < bufsize/2){
        // добавляет отсчеты в серию, пока не наткнется на отсчеты ниже порога
        /*
         * start - флаг начала сигнала
         * thrsh_cou - счетчик отсчетов ниже порога
         * max_noise_samples - максимальное кол-во шумовых низкоуровневых отсчетов после начала сигнала
         * достижение max_noise_samples означает, что сигнала закончился
         */
        unsigned start_offset = offset;
        if(!prev){
            for(auto [start, thrsh_cou] = std::make_tuple(false, 0);
                (offset-start_offset+1 < maxsamp) && (thrsh_cou < max_end_noise_samples) && (offset < bufsize/2);
                offset++){
                qint16 samp = cuza.getSample(offset);
                if(abs(samp) >= THRESHOLD && thrsh_cou != max_noise_samples){
                    thrsh_cou = 0;
                    if(!start) start = true;
                }
                if(abs(samp) < THRESHOLD && start){
                    thrsh_cou++;
                    if(thrsh_cou == max_end_noise_samples){
                        /* проверяем средним скользящим окном есть ли наличие сигнала
                         * в выведенных отсчетах, потому что мог быть просто единичный выброс
                         * SWEEP_WINDOWS - максимум проходок
                         * meanwin_step - шаг по отсчетам
                         * signal - признак наличия сигнала */
                        const double meanwin_step = (offset - start_offset + 1)/SWEEP_WINDOWS;
                        bool signal = false;
                        for(unsigned i = 0; i < SWEEP_WINDOWS && !signal; i++){
                            unsigned sum = 0;
                            for(unsigned j = start_offset + i*meanwin_step; j < start_offset + (i+1)*meanwin_step; j++)
                                sum += abs(cuza.getSample(j));
                            if(sum/meanwin_step >= THRESHOLD) signal = true;
                        }
                        if(!signal){
                            thrsh_cou = 0;
                            start = 0;
                        }
                    }
                }
                m_lineseries[Chart::SAMPS]->append(offset/*(1.0/sampling_rate)*/, samp);
            }
        } else {
                for(auto [start, thrsh_cou] = std::make_tuple(false, 0);
                    (start_offset - offset + 1 < maxsamp) && (thrsh_cou < max_end_noise_samples);
                    offset--){
                    if(offset == UINT_MAX){
                        // если достигнуто начало буфера
                        offset = bufsize/2-1;
                        start_offset = bufsize/2-1;
                        m_lineseries[0]->clear();
                        m_lineseries[1]->clear();
                    }
                    qint16 samp = cuza.getSample(offset);
                    if(abs(samp) >= THRESHOLD && thrsh_cou != max_noise_samples){
                        thrsh_cou = 0;
                        if(!start) start = true;
                    }
                    if(abs(samp) < THRESHOLD && start){
                        thrsh_cou++;
                        if(thrsh_cou == max_end_noise_samples){
                            /* проверяем средним скользящим окном есть ли наличие сигнала
                             * в выведенных отсчетах, потому что мог быть просто единичный выброс
                             * SWEEP_WINDOWS - максимум проходок
                             * meanwin_step - шаг по отсчетам
                             * signal - признак наличия сигнала */
                            const double meanwin_step = (start_offset - offset + 1)/SWEEP_WINDOWS;
                            bool signal = false;
                            for(unsigned i = 0; i < SWEEP_WINDOWS && !signal; i++){
                                unsigned sum = 0;
                                for(unsigned j = offset + i*meanwin_step;
                                    (j < offset + (i+1)*meanwin_step) && j < bufsize/2; j++)
                                    sum += abs(cuza.getSample(j));
                                if(sum/meanwin_step >= THRESHOLD) signal = true;
                            }
                            if(!signal){
                                thrsh_cou = 0;
                                start = 0;
                            }
                        }
                    }
                    m_lineseries[Chart::SAMPS]->append(offset/*(1.0/sampling_rate)*/, samp);
                }
            }
        // преобразование гильберта
        const unsigned start_samp = !prev ? start_offset : offset,
                end_samp = !prev ? offset : start_offset,
                size = end_samp-start_samp+1;
        std::vector<qint16> env;
        envelope(env, start_samp, end_samp);
//        for(unsigned i = start_samp; i <= end_samp; i++)
//            m_lineseries[Chart::ENV]->append(i*(1.0/sampling_rate), env[i-start_samp]);
    }
#endif
//    for(unsigned short i = 0; i < cuza.getSeriesCount(); i++)
        m_chart->addSeries(m_lineseries[0]);
    m_chart->createDefaultAxes();
    m_chart->axisY()->setRange(-2048, 2048);
    m_chart->legend()->markers()[Chart::SAMPS]->setLabel("Сигнал");
//    m_chart->legend()->markers()[Chart::ENV]->setLabel("Огибающая");
}

unsigned DataProcessor::getScale() const
{
    return scale;
}

void DataProcessor::resizeCheck(const unsigned len, const unsigned width)
{
    if(len*scale< width)
        scale = width/(double)(len);
}

complex_ptr DataProcessor::dft(const std::vector<qint16>& in, const unsigned N)
{
    complex *dft_ptr = new complex[N];
    for(unsigned i = 0; i < N; i++)
        for(unsigned j = 0; j < N; j++){
            double arg = 2*M_PI*j*i/(double)N;
            complex c(in[j]*cos(arg), -in[j]*sin(arg));
            dft_ptr[i] += c;
        }
    return complex_ptr(dft_ptr);
}

complex_ptr DataProcessor::inv_dft(const complex_ptr spectrum, const unsigned N)
{
    complex* inv_dft = new complex[N];
    for(unsigned i = 0; i < N; i++)
        for(unsigned j = 0; j < N; j++){
            double arg = 2*M_PI*j*i/(double)N;
            inv_dft[i] += spectrum.get()[j]*complex(cos(arg)/N, sin(arg)/N);
        }
    return complex_ptr(inv_dft);
}

complex_ptr DataProcessor::fft(const std::vector<qint16>& in, const unsigned N)
{
    // алгоритм Кули-Тьюки
    if(N == 2) return dft(in, N);
    complex_ptr fft_ptr     =    fft(std::vector<qint16>(in.begin(), in.end()-N/2), N/2),
                fft_ptr_2   =    fft(std::vector<qint16>(in.begin()+N/2, in.end()), N/2);
    complex*    fft_res = new complex[N/2];
    // объединение
    for(unsigned i = 0; i < N/2; i++){
        fft_res[i] = fft_ptr.get()[i*2]+fft_ptr_2.get()[i*2];
    }
    return complex_ptr(fft_res);
}


complex_ptr DataProcessor::inv_fft(const complex_ptr spectrum, const unsigned N)
{
    // неправильно считается
    complex_ptr fft_ptr (new complex[N]);
    for(unsigned i = 0; i < N; i++){
        complex sum(0,0);
        for(unsigned j = 0; j < N/2; j++){
            double arg = j*2*i*2*M_PI/(double)N;
            sum += complex(cos(arg)/N, -sin(arg)/N);
        }
        fft_ptr.get()[i] = sum;
    }
    return fft_ptr;
}

complex_ptr DataProcessor::hilbert(const std::vector<qint16>& in, const unsigned N)
{
    complex_ptr fft_ptr = fft(in, N);

    return inv_fft(dft_ptr, N);
}

void DataProcessor::envelope(std::vector<qint16>& out, const unsigned start, const unsigned end)
{
    Cuza& cuza = Cuza::get();
    unsigned size = end-start+1;
    if(size%2) {
        out.resize(++size);
        out[end+1] = 0;
    } else out.resize(size);
    for(unsigned i = start; i <= end; i++)
        out[i-start] = cuza.getSample(i);
    complex_ptr hil = hilbert(out, size);
    for(unsigned i = 0; i < end-start+1; i++)
        out[i] = abs(hil.get()[i]);
}

void DataProcessor::setScale(const double& value)
{
    Q_ASSERT(value > 0);
    const unsigned couBuf = NCOUNT*bufLen;
}
