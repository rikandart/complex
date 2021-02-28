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

void DataProcessor::oscOutput(QLineSeries ***series, QChart* chart, bool prev)
{
    Q_ASSERT(series != nullptr || m_lineseries != nullptr);
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
    for(unsigned short i = 0; i < cuza.getSeriesCount(); i++){
        qDebug() << m_lineseries[i];
        (*(m_lineseries[i])) = new QLineSeries;
        // 260221 - почему-то не создается новое место в куче
        qDebug() << "here";
        qDebug() << (*m_lineseries[i]);
        (*(m_lineseries[i]))->clear();
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
                (*m_lineseries[Chart::SAMPS])->append(offset*(1.0/sampling_rate), samp);
                (*m_lineseries[Chart::ENV])->append(offset*(1.0/sampling_rate), abs(samp));
            }
        } else {
                for(auto [start, thrsh_cou] = std::make_tuple(false, 0);
                    (start_offset - offset + 1 < maxsamp) && (thrsh_cou < max_end_noise_samples);
                    offset--){
                    if(offset == UINT_MAX){
                        // если достигнуто начало буфера
                        offset = bufsize/2-1;
                        start_offset = bufsize/2-1;
                        (*m_lineseries[0])->clear();
                        (*m_lineseries[1])->clear();
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
                    (*m_lineseries[Chart::SAMPS])->append(offset*(1.0/sampling_rate), samp);
                    (*m_lineseries[Chart::ENV])->append(offset*(1.0/sampling_rate), abs(samp));
                }
            }
    }
#endif
    for(unsigned short i = 0; i < cuza.getSeriesCount(); i++)
        m_chart->addSeries((*m_lineseries[i]));
    m_chart->createDefaultAxes();
    m_chart->axisY()->setRange(-2048, 2048);
    m_chart->legend()->markers()[0]->setLabel("Сигнал");
    m_chart->legend()->markers()[0]->setLabel("Огибающая");
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

void DataProcessor::setScale(const double& value)
{
    Q_ASSERT(value > 0);
    const unsigned couBuf = NCOUNT*bufLen;
}
