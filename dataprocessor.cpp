#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){
}

DataProcessor::~DataProcessor(){}

void DataProcessor::Read(const QString &filename)
{
    offset = end_offset = 0;
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
    const int   sampling_rate = cuza.getFd(),
                maxsamp = cuza.getMaxVisSamples(),
                bufsize = cuza.getBufferSize(),
                lbound = !prev ? 0 : bufsize/2-1,
                ubound = !prev ? bufsize/2-1 : 0,
                win_len = 20, // длина окна для поиска начала сигнала
                sign = 1 - prev*2,  // (1 - prev*2) - получение знака из bool
                                    // -1 в обратную сторону +1 - прямо
                wl_offset = win_len * sign;
    const double tuned_freq = cuza.getFreq();
    static bool next = false;
#ifndef SPECTRUM_MAX_SEARCH
    // назначение сдвига в зависимости от направления движения по записи
    static unsigned prev_offset = offset;
    if(prev){
        if(!next) // если был предыдущий сигнал
            prev_offset = offset ? offset : bufsize/2-1;
        else offset = prev_offset ? prev_offset : bufsize/2-1;
    } else {
        if(!next) // если был предыдущий сигнал
            offset = (prev_offset >= bufsize/2-1) ? 0 : prev_offset;
        else {
            offset = (offset >= bufsize/2-1) ? 0 : offset;
            prev_offset = offset;
        }
    }
    const unsigned short max_noise_samples = 10,
                         max_end_noise_samples = 10*max_noise_samples;
#elif defined(SPECTRUM_MAX_SEARCH)
    // если перемещаемся только вправо или только влево
    // !prev && next || prev & !next
    if(prev ^ next) offset = end_offset;
    // в обратном же случае сдвиг не меняется
#endif
    next = !prev;
#ifdef MAX_SAMP
    for(unsigned i = 0; i < 2048/*cuza.getMaxVisSamples()*/; i++){
            m_lineseries[ChartType::chOSC]->append(i, cuza.getSample(i));
    }
    m_charts[ChartType::chOSC]->addSeries(m_lineseries[ChartType::chOSC]);
    m_charts[ChartType::chOSC]->createDefaultAxes();
#elif defined(SPECTRUM_MAX_SEARCH)
#define INC(i) i += sign
#define COMPARE(i,j) (int)(i)*sign < (int)(j)*sign
    auto isInTheRange = [&](int offset, bool off_end)->bool{
        return sign*offset < (ubound-(!off_end)*win_len)/* && sign*offset >= sign*lbound*/;
    };
    /*отступы в отсчетах*/
    auto margin = [&](int offset, bool off_end = true, int end_offset = 0)->int{
        const short i_max = 3;
        // i_max+1 потому что в цкиле for происходит уменьшение
        int i = !off_end ? i_max+1 : 0;
        if(!off_end){
            for(bool stop = false; !stop && i > 0; i--)
                stop = (unsigned)abs(offset-end_offset) >= win_len*i;
        } else {
            // THRESHOLD+1 чтобы цикл сразу не завершился
            fftw_complex *sig = new fftw_complex[win_len],
                         *spect = new fftw_complex[win_len];
                for(int spec_max = 0;
                    spec_max <= THRESHOLD && i < i_max && isInTheRange(offset, off_end); i++){
                    spec_max = 0;
                    for(int j = offset+wl_offset*i;
                        COMPARE(j, offset+wl_offset*(i+1)) && isInTheRange(j, off_end); INC(j)){
                        sig[sign*j-sign*(offset+wl_offset*i)][REAL] = cuza.getSample(j);
                        sig[sign*j-sign*(offset+wl_offset*i)][IMAG] = 0;
                    }
                    fft(sig,spect,win_len);
                    for(unsigned j = 0; j  < win_len; j++){
                        double spec_samp = abs_complex(sig[j]);
                        if(spec_max < spec_samp) spec_max = spec_samp;
                    }
                }
            delete [] sig; delete [] spect;
        }
        if(!off_end) i = -i;
        return  wl_offset*i;
    };
    // проверка на то, что сдвиг находится рядом с верхней границей
    auto checkBounds = [&]()->void{
        if(sign*offset >= (ubound-win_len)) offset = end_offset = lbound;
    };
    checkBounds();
    // поиск по максимум спектра в окне
    // после start_samp 18300 end_samp 30180 идет бесконечный поиск
    auto findBound = [&](int& offset, bool off_end)->void{
        fftw_complex *sig = new fftw_complex[win_len], *spect = new fftw_complex[win_len];
        int sgn = 1-off_end*2;
        for(int i = offset, spec_max = (THRESHOLD+1)*off_end, approve = 0, tmp_offset = 0;
            // offset*(sgn+1)/2+i*(1-sgn)/2
            // если !off_end, то передается сдвиг, если off_end, то передается i
            (approve < 3) && COMPARE(i, offset + wl_offset) &&
            isInTheRange(offset*(1+sgn)/2+i*(1-sgn)/2, off_end);
            INC(i)){
            sig[sign*i-sign*offset][REAL] = cuza.getSample(i);
            sig[sign*i-sign*offset][IMAG] = 0;
            if(i == offset + wl_offset - sign){
                fft(sig, spect, win_len);
                spec_max = 0;
                for(unsigned j = 0; j < win_len; j++){
                    double spec_samp = abs_complex(spect[j]);
                    if(spec_max < spec_samp)
                        spec_max = spec_samp;
                }
                if(sgn*spec_max < sgn*THRESHOLD){
                    approve = 0;
                    offset += wl_offset;
                    checkBounds();
                    i = offset+1;
                } else {
                    // если 3 окна подряд продолжается sgn*spec_max > sgn*THRESHOLD
                    // то значение сдвига было найдено верно
                    if(approve == 0) tmp_offset = offset;
                    approve++;
                    if(sign*offset < ubound-win_len){
                        offset += wl_offset;
                        checkBounds();
                        i = offset+1;
                        if(approve == 3) offset = tmp_offset;
                    }
                }
            }
        }
        delete [] sig; delete [] spect;
    };
    findBound(offset, false);
    int tmp_offset = offset;
    offset += margin(offset, false, end_offset);
    end_offset = tmp_offset;
    findBound(end_offset, true);
    end_offset += margin(end_offset);
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
#endif
    // если не был достигнут конец буфера
    if(offset < bufsize/2){
#ifndef SPECTRUM_MAX_SEARCH
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
//                m_lineseries[Series::SAMPS]->append((offset-start_offset)/**(1.0/sampling_rate)*/, cuza.getSample(offset-1)/2.048);
//                m_lineseries[Series::SAMPS]->append((offset-start_offset)/**(1.0/sampling_rate)*/, samp/2.048);
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
//                    m_lineseries[Series::SAMPS]->append((start_offset-offset)/**(1.0/sampling_rate)*/, cuza.getSample(offset+1)/2.048);
//                    m_lineseries[Series::SAMPS]->append((start_offset-offset)/**(1.0/sampling_rate)*/, samp/2.048);
                }
            }
        // вычисление квадратуры и бпф
#endif
#define CLASSIC
#ifdef CLASSIC
#ifndef SPECTRUM_MAX_SEARCH
        const unsigned start_samp = !prev ? start_offset : offset,
                end_samp = !prev ? offset : start_offset,
                size = end_samp-start_samp+1;
#else
        const unsigned start_samp = !prev ? offset : end_offset,
                end_samp = !prev ? end_offset : offset,
                size = end_samp-start_samp+1;
        qDebug() << "start_samp" << start_samp << "end_samp" << end_samp;
#endif
#else
    const unsigned start_samp = 1688547, end_samp = 1690110,
            size = end_samp-start_samp+1;
#endif
        fftw_complex    *in_samps = new fftw_complex[size],
                        *fft_res = new fftw_complex[size],
                        *complex_sig = new fftw_complex[size];
        double          *phase = new double[size],
                        *freq = new double[size-1];
        for(unsigned i = start_samp; i <= end_samp; i++){
            in_samps[i-start_samp][REAL] = cuza.getSample(i)/ADC_COEF;
            in_samps[i-start_samp][IMAG] = 0;
        }
        for(unsigned i = 0; i < cuza.getChartCount(); i++){
            if(i != ChartType::chAMP) emit setPointsVecSize(size, ChartType(i));
            else emit setPointsVecSize(size/2+1, ChartType(i));
        }
        calc_fft_comp_sig(fft_res, complex_sig, start_samp, end_samp, in_samps);
        calc_phase(complex_sig, phase, size);
        calc_instfreq(phase, freq, size);
        double *phase_max = std::max_element(phase, phase+size),
               *freq_max = std::max_element(freq, freq+size-1);
        unsigned phase_coef = 1, freq_coef = 1;
        // расчет коэффициента масштаба для фазы (вывод на экран не больше 3х цифр)
        // это нееобходимо, чтобы длина цифр не влияла на размер рисунков графиков
        // и для согласованности
        for(bool stop = false; !stop; !stop ? phase_coef*=10 : phase_coef*=1)
            stop = abs(int(*phase_max/phase_coef)) < 1000;
        for(bool stop = false; !stop; !stop ? freq_coef*=10 : freq_coef*=1)
            stop = abs(int(*freq_max/freq_coef)) < 1000;
        emit setCoef(phase_coef, ChartType::chPHASE);
        emit setCoef(freq_coef, ChartType::chFREQ);
        for(unsigned i = 0; i < size; i++){
            if(i > 0){
                m_lineseries[Series::SAMPS]->append((i+start_samp)/**(1.0/sampling_rate)*/, in_samps[i-1][REAL]);
                m_lineseries[Series::ENV]->append((i+start_samp)/**(1.0/sampling_rate)*/, abs_complex(complex_sig[i-1]));
            }
            m_lineseries[Series::SAMPS]->append((i+start_samp)/**(1.0/sampling_rate)*/, in_samps[i][REAL]);
            m_lineseries[Series::ENV]->append((i+start_samp)/**(1.0/sampling_rate)*/, abs_complex(complex_sig[i]));
            m_lineseries[Series::PHASE]->append((i+start_samp)/**(1.0/sampling_rate)*/, phase[i]/phase_coef);
            if(i > 0)
                m_lineseries[Series::FREQ]->append((i+start_samp)/**(1.0/sampling_rate)*/, freq[i-1]/freq_coef);
            if(i <= size/2){
                if(i != 0)
                    m_lineseries[Series::AMP]->append(tuned_freq - sampling_rate/2.0 + i*sampling_rate/(double)size, abs_complex(fft_res[i-1+size/2]));
                m_lineseries[Series::AMP]->append(tuned_freq - sampling_rate/2.0 + i*sampling_rate/(double)size, abs_complex(fft_res[i+size/2]));
            }
        }
        delete[] in_samps; delete[] fft_res; delete[] complex_sig; delete[]  phase; delete[] freq;
    }
    QString series_name[] = {"Сигнал", "Фаза", "Мгновенная частота", "Амплитудный спектр", "Огибающая"};
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
    // axes()[0] - x axis, axes()[1] - y
    double osc_center = [](const QValueAxis* axis)->double{
        return (axis->max()-axis->min())/2.0;
    }((QValueAxis*)m_charts[ChartType::chOSC]->axes()[1]);
    m_charts[ChartType::chOSC]->axes()[1]->setRange(-osc_center, osc_center);
//    m_charts[ChartType::chPHASE]->axes()[1]->setRange(-90,90);
    m_charts[ChartType::chAMP]->axes()[1]->setMin(0);
    m_charts[ChartType::chFREQ]->axes()[0]->setMin(((QValueAxis*)m_charts[ChartType::chOSC]->axes()[0])->min());
    qDebug() << "-----------------------------------------------";
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

void DataProcessor::calc_instfreq(const double *phase, double *out, const unsigned size)
{
    const double sampling_rate = Cuza::get().getFd(),
                 tuned_freq = Cuza::get().getFreq();
    for(unsigned i = 0; i < size-1; i++)
        out[i] = tuned_freq - abs(phase[i+1]-phase[i]);
}

void DataProcessor::calc_phase(fftw_complex *complex_sig, double *out, const unsigned size)
{
    auto sign = [](const double x)->int{
        return x/(x ?abs(x) : 1);
    };
    // коэффициент для умножения на пи
    int N_PI = 0;
    const double thresh = M_PI + 0*M_PI/180;
    for (unsigned i = 0; i < size; i++){
        out[i] = atan2(complex_sig[i][IMAG], complex_sig[i][REAL]);
#define THRESH
#ifdef  THRESH
        if(i > 1){
        // (out[i-1]/180-N_PI)*M_PI - приведение к текущим радианам из градусов
            if(abs(out[i]-((out[i-1]/180-N_PI)*M_PI)) > thresh){
                if(out[i]> (out[i-1]/180-N_PI)*M_PI) N_PI -= 2;
                else N_PI += 2;
            }
        }
#endif
        out[i] += N_PI*M_PI;
        out[i] *= 180/M_PI;
    }
}

void DataProcessor::setScale(const double& value)
{
    Q_ASSERT(value > 0);
    const unsigned couBuf = NCOUNT*bufLen;
}
