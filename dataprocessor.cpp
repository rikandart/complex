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

void DataProcessor::oscOutput(QLineSeries** &series, QChart** charts, bool prev)
{
    Q_ASSERT(series != 0 || m_lineseries != nullptr);
    Q_ASSERT(charts != nullptr || m_charts != nullptr);
    Cuza& cuza = Cuza::get();
    if(charts) m_charts = charts;
    if(series) m_lineseries = series;
    // ремув убирает все данные о выведенных точках на график, а также
    // почему-то удаляет все данные о точках из кучи
    for(unsigned i = 0; i < cuza.getChartCount(); i++) m_charts[i]->removeAllSeries();
    m_lineseries = new QLineSeries*[Cuza::get().getSeriesCount()];
    for(unsigned short i = 0; i < cuza.getSeriesCount(); i++)
        (m_lineseries[i] = new QLineSeries)->clear();

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
//#define MAX_SAMP
#ifdef MAX_SAMP
    for(unsigned i = 0; i < cuza.getMaxVisSamples(); i++){
            m_lineseries[Chart::SAMPS]->append(i, cuza.getSample(i));
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
                m_lineseries[Series::SAMPS]->append(offset/**(1.0/sampling_rate)*/, samp);
            }
        } else {
                for(auto [start, thrsh_cou] = std::make_tuple(false, 0);
                    (start_offset - offset + 1 < maxsamp) && (thrsh_cou < max_end_noise_samples);
                    offset--){
                    if(offset == UINT_MAX){
                        // если достигнуто начало буфера
                        offset = bufsize/2-1;
                        start_offset = bufsize/2-1;
                        m_lineseries[Series::SAMPS]->clear();
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
                    m_lineseries[Series::SAMPS]->append(offset/**(1.0/sampling_rate)*/, samp);
                }
            }
        // преобразование гильберта
        const unsigned start_samp = !prev ? start_offset : offset,
                end_samp = !prev ? offset : start_offset,
                size = end_samp-start_samp+1;
        fftw_complex fft_res[size], complex_sig[size];
        calc_fft_comp_sig(fft_res, complex_sig, start_samp, end_samp);
        auto abs = [](const fftw_complex& fft_samp)->double{
                return sqrt(pow(fft_samp[REAL], 2) + pow(fft_samp[IMAG], 2));
        };
        auto arg = [](const fftw_complex& fft_samp)->double{
            return atan(fft_samp[IMAG]/fft_samp[REAL])*180/M_PI;
        };
        for(unsigned i = start_samp; i <= end_samp; i++){
            m_lineseries[Series::ENV]->append(i/**(1.0/sampling_rate)*/, abs(complex_sig[i-start_samp]));
            m_lineseries[Series::PHASE]->append(i*(1.0/*/sampling_rate*/),arg(complex_sig[i-start_samp]));
            m_lineseries[Series::AMP]->append(i*(1.0/*/sampling_rate*/), abs(fft_res[i-start_samp])/100);
        }
        qDebug() << "-----------------------------------------------";
    }
#endif
    QString series_name[] = {"Сигнал", "Фаза", "Амплитудный спектр", "Огибающая"};
    for(unsigned short i = 0; i < cuza.getChartCount(); i++){
        m_charts[i]->addSeries(m_lineseries[i]);
        m_charts[i]->createDefaultAxes();
        m_charts[i]->legend()->markers()[0]->setLabel(series_name[i]);
        if(i == Chart::chOSC){
            m_charts[i]->addSeries(m_lineseries[i+3]);
            m_charts[i]->legend()->markers()[1]->setLabel(series_name[i+3]);
            m_charts[i]->createDefaultAxes();
//            m_charts[i]->axisY()->setRange(-2048, 2048);
        }
    }
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

complex_ptr DataProcessor::dft(const std::vector<complex>& in, const unsigned N)
{
    complex *dft_ptr = new complex[N];
    for(unsigned i = 0; i < N; i++)
        for(unsigned j = 0; j < N; j++){
            double arg = 2*M_PI*j*i/(double)N;
            dft_ptr[i] += in[j]*cos(arg), -in[j]*sin(arg);
        }
    return complex_ptr(dft_ptr);
}

void DataProcessor::fft(fftw_complex *in, fftw_complex *out, const unsigned N)
{
    // create a DFT plan
    fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    // execute the plan
    fftw_execute(plan);
    // do some cleaning
    fftw_destroy_plan(plan);
    fftw_cleanup();
}

void DataProcessor::ifft(fftw_complex *in, fftw_complex *out, const unsigned N)
{
    // create an IDFT plan
    fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    // execute the plan
    fftw_execute(plan);
    // do some cleaning
    fftw_destroy_plan(plan);
    fftw_cleanup();
    // scale the output to obtain the exact inverse
    for (int i = 0; i < N; ++i) {
        out[i][REAL] /= N;
        out[i][IMAG] /= N;
    }
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

void DataProcessor::hilbert(fftw_complex* in, fftw_complex* out, const unsigned N)
{
#define FFT
#ifdef FFT
    auto assign_compl = [](fftw_complex& dest, fftw_complex& val, unsigned coef = 1)->void{
        dest[REAL] = val[REAL]*coef;
        dest[IMAG] = val[IMAG]*coef;
    };
    auto assign_compl_d = [](fftw_complex& dest, double val)->void{
        dest[REAL] = val;
        dest[IMAG] = val;
    };
    fftw_complex in_scaled[N];
    unsigned half = round(N/2.);
    assign_compl(in_scaled[0], in[0]);
    assign_compl(in_scaled[0], in[0]);
    for(unsigned i = 1; i < N; i++){
        if(i < half)  assign_compl(in_scaled[i], in[i], 2);
        else if (i > half) assign_compl_d(in_scaled[i], 0);
    }
//    memset можно не использовать, т.к. при объявлении in_scaled in_scaled[i] == 0
//    memset(&in_scaled[half+1], 0, (!(half%2) ? half : (half-1))*sizeof(fftw_complex));
    ifft(in_scaled, out, N);
#else
    complex_ptr dft_ptr = dft(in, N);
    for(unsigned i = 1; i < N/2; i++)
        dft_ptr.get()[i] *= 2;
    memset(&dft_ptr.get()[N/2+1], 0, N/2-1);
    return inv_dft(dft_ptr, N);
#endif
}

void DataProcessor::calc_fft_comp_sig(fftw_complex* fft_res, fftw_complex* complex_sig, const unsigned start, const unsigned end)
{
    Cuza& cuza = Cuza::get();
    const unsigned N = end-start+1;
    fftw_complex in[N];
    for(unsigned i = start; i <= end; i++){
        in[i-start][REAL] = cuza.getSample(i);
        in[i-start][IMAG] = 0;
    }
    fft(in, fft_res, end-start+1);
    hilbert(fft_res, complex_sig, N);
}

void DataProcessor::setScale(const double& value)
{
    Q_ASSERT(value > 0);
    const unsigned couBuf = NCOUNT*bufLen;
}
