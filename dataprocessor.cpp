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
    const double tuned_freq = cuza.getFreq();
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
                // в мВ
                m_lineseries[Series::SAMPS]->append((offset-start_offset)/**(1.0/sampling_rate)*/, cuza.getSample(offset-1)/2.048);
                m_lineseries[Series::SAMPS]->append((offset-start_offset)/**(1.0/sampling_rate)*/, samp/2.048);
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
                    // в мВ
                    m_lineseries[Series::SAMPS]->append((start_offset-offset)/**(1.0/sampling_rate)*/, cuza.getSample(offset+1)/2.048);
                    m_lineseries[Series::SAMPS]->append((start_offset-offset)/**(1.0/sampling_rate)*/, samp/2.048);
                }
            }
        // вычисление квадратуры и бпф
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
        for(unsigned i = 0; i < size; i++){
            if(i != 0)
                m_lineseries[Series::ENV]->append(i/**(1.0/sampling_rate)*/, abs(complex_sig[i-1]));
            m_lineseries[Series::ENV]->append(i/**(1.0/sampling_rate)*/, abs(complex_sig[i]));
            m_lineseries[Series::PHASE]->append(i/**(1.0/sampling_rate)*/,arg(complex_sig[i]));
//            if(i != start_samp)
//                m_lineseries[Series::FREQ]->append(i/**(1.0/sampling_rate)*/,
//                                                   (arg(complex_sig[i])-arg(complex_sig[i-1]))*sampling_rate);
            if(i <= size/2){
                if(i != 0)
                    m_lineseries[Series::AMP]->append(tuned_freq - sampling_rate/2.0 + i*sampling_rate/(double)size, abs(fft_res[i-1]));
                m_lineseries[Series::AMP]->append(tuned_freq - sampling_rate/2.0 + i*sampling_rate/(double)size, abs(fft_res[i]));
            }
        }
        qDebug() << "tune_frequency" << tuned_freq;
        qDebug() << "-----------------------------------------------";
    }
    QString series_name[] = {"Сигнал", "Фаза", /*"Мгновенная частота",*/ "Амплитудный спектр", "Огибающая"};
    for(unsigned short i = 0; i < cuza.getChartCount(); i++){
        m_charts[i]->addSeries(m_lineseries[i]);
        m_charts[i]->createDefaultAxes();
        m_charts[i]->legend()->markers()[0]->setLabel(series_name[i]);
        if(i == ChartType::chOSC){
            m_charts[i]->addSeries(m_lineseries[i+cuza.getSeriesCount()-1]);
            m_charts[i]->legend()->markers()[1]->setLabel(series_name[i+cuza.getSeriesCount()-1]);
            m_charts[i]->createDefaultAxes();
        }
    }
#endif
    // axes()[0] - x axis, axes()[1] - y
    double osc_center = [](const QValueAxis* axis)->double{
        return (axis->max()-axis->min())/2.0;
    }((QValueAxis*)m_charts[ChartType::chOSC]->axes()[1]);
    m_charts[ChartType::chOSC]->axes()[1]->setRange(-osc_center, osc_center);
    m_charts[ChartType::chPHASE]->axes()[1]->setRange(-90,90);
    m_charts[ChartType::chAMP]->axes()[1]->setMin(0);
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

void DataProcessor::dft(fftw_complex *in, fftw_complex *out, const unsigned N)
{
    for(unsigned i = 0; i < N; i++){
        out[i][REAL] = out[i][IMAG] = 0;
        for(unsigned j = 0; j < N; j++){
            double arg = 2*M_PI*j*i/(double)N;
            out[i][REAL] += in[j][REAL]*cos(arg);
            out[i][IMAG] += -in[j][REAL]*sin(arg);
        }
    }
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

void DataProcessor::idft(fftw_complex *in, fftw_complex *out, const unsigned N)
{
    for(unsigned i = 0; i < N; i++)
        for(unsigned j = 0; j < N; j++){
            double arg = 2*M_PI*j*i/(double)N;
            out[i][REAL] += in[j][REAL]*cos(arg)/N;
            out[i][IMAG] += in[j][IMAG]*sin(arg)/N;
        }
}

void DataProcessor::hilbert(fftw_complex* in, fftw_complex* out, const unsigned N)
{

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
    ifft(in_scaled, out, N);
}

void DataProcessor::calc_fft_comp_sig(fftw_complex* fft_res, fftw_complex* complex_sig,
                                      const unsigned start, const unsigned end, fftw_complex* input)
{
    Cuza& cuza = Cuza::get();
    const unsigned N = end-start+1;
    fftw_complex in[N];
    for(unsigned i = start; i <= end; i++){
        int samp = 0;
        if(input == nullptr) samp = cuza.getSample(i)/2.048;
        else samp = input[i-start][REAL];
        in[i-start][REAL] = samp;
        in[i-start][IMAG] = 0;
    }
    fft(in, fft_res, N);
    hilbert(fft_res, complex_sig, N);
}

void DataProcessor::setScale(const double& value)
{
    Q_ASSERT(value > 0);
    const unsigned couBuf = NCOUNT*bufLen;
}
