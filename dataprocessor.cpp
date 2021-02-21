#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){
}

DataProcessor::~DataProcessor(){}

void DataProcessor::Read(const QString &filename)
{
    win_i = win_offset = 0;
    // считать лучше весь файл
    // длина окна определяется по поиску следующего сигнала синхронизации
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream d_stream(&file);
    Cuza& cuza = Cuza::get();
    unsigned win_len = cuza.getSampWinLen();
//    cuza.cleanMainBuffer();
//    if(filename.compare(cuza.getFilename(), Qt::CaseSensitive)){

//    }
    unsigned f_len = (file.size() >= cuza.getBuffLength()) ?
                      cuza.getBuffLength() : file.size();
//    cuza.setFilename(filename);
    cuza.setWinCount(f_len/(2.0*win_len));
    cuza.resizeBuffer(f_len);
//    d_stream.skipRawData(cuza.incWinIndex()*2*win_len);
    for(unsigned i = 0; i < f_len; i++){
        uchar element;
        d_stream >> element;
        cuza.appendToBuffer(element);
    }
//    cuza.retrieveSync();
//    cuza.retriveWinLen();
    cuza.retrieveSamples();
//    cuza
}

void DataProcessor::oscOutput(QLineSeries **series, QChart* chart, bool prev)
{
    Q_ASSERT(series != nullptr || m_lineseries != nullptr);
    Q_ASSERT(chart != nullptr || m_chart != nullptr);
    Cuza& cuza = Cuza::get();
    if(chart) m_chart = chart;
    if(series) m_lineseries = series;
    // ремув убирает все данные о выведенных точках на график, а также
    // почему-то удаляет все данные о точках из кучи
    m_chart->removeAllSeries();
    (*m_lineseries) = new QLineSeries;
    (*m_lineseries)->clear();
    // поиск сигнала по порогу
    const unsigned  sampling_rate = cuza.getFd(),
                    maxsamp = cuza.getMaxVisSamples(),
                    bufsize = cuza.getBufferSize(),
                    win_len = cuza.getSampWinLen(),
                    win_count = cuza.getWinCount();
    if(win_offset >= bufsize/2) win_offset = 0;
    if(win_i >= win_count) win_i = 0;
    if(prev){
        if(!win_offset) win_offset = bufsize/2;
        win_i = win_offset/win_len;
    }
    const unsigned short max_noise_samples = 10;
#define MAX_SAMP
#ifndef MAX_SAMP
    for(unsigned i = 0; i < cuza.getMaxVisSamples(); i++){
            (*m_lineseries)->append(i, cuza.getSample(i));
    }
#else
    // поиск значения, превышающего порог
    for(bool start = 0;
        !start && (!prev ? win_offset < bufsize/2 - win_len : win_offset > 0);
        !prev ? win_i++ : win_i--){
        win_offset = win_i*win_len;
        unsigned next_offset = (!prev ? (win_i+1) : (win_i-1)) * win_len;
        if(next_offset >= bufsize/2) next_offset = 0;
        // поиск по всем отсчетам окна
        // если есть флаг prev, то поиск проходит в другую сторону
        for(unsigned i = !prev ? win_offset : win_offset-1;
            (!prev ? i < next_offset : i >= next_offset) && !start; !prev ? i++ : i--){
            if(abs(cuza.getSample(i)) > THRESHOLD){
                start = 1;
                // чтобы начиналось не сразу с сигнала
                if(win_offset != 0 && !(i%win_offset)) {
                    for(unsigned tmp_i = win_offset, thrsh_cou = 0; thrsh_cou < 15*max_noise_samples; tmp_i--){
                        int samp = cuza.getSample(tmp_i);
                        if(abs(samp) > THRESHOLD){
                            thrsh_cou = 0;
                        } else {
                            thrsh_cou++;
                        }
                        if(thrsh_cou >= max_noise_samples) win_offset = tmp_i;
                        if(!(win_offset % win_len)) win_i = win_offset/win_len;
                        if(win_i == UINT_MAX) win_i = 0;
                    }
                    qDebug() << "offset- 2" << "win_offset" << win_offset << "win_i" << win_i;
                }
//                qDebug() << "osc start point" << "win_i" << win_i << "win_offset" << win_offset <<
//                            "bufsize" << bufsize << "sample at" << i;
            }
        }
    }
    // если не был достигнут конец буфера
    if(win_offset < bufsize/2 - win_len){
        // добавляет отсчеты в серию, пока не наткнется на окно, где отсчеты упадут ниже порога
        /*
         * start - флаг начала сигнала
         * thrsh_cou - счетчик отсчетов ниже порога
         * delta - разница между концом окна и индексом текущего сэмпла
         * max_noise_samples - максимальное кол-во шумовых низкоуровневых отсчетов после начала сигнала
         * достижение max_noise_samples означает, что сигнала закончился */
        const unsigned start_win_offset = win_offset;
        if(!prev){
            for(auto [i, start, thrsh_cou, delta] =
                std::make_tuple(win_offset, false, 0, 0);
                (i-start_win_offset < maxsamp) && (thrsh_cou >= max_noise_samples) ? delta > 1 : 1; i++){
                qint16 samp = cuza.getSample(i);
                // если конец окна, то меняем сдвиг
                if(i % win_len == win_len-1) win_offset = (++win_i)*win_len;
                // 21022021 отвязать оффсет от окон
                /*if(thrsh_cou >= max_noise_samples && abs(samp) >= THRESHOLD){
                    delta = 1;
                    win_offset = i;
                    continue;
                }*/
                if(abs(samp) >= THRESHOLD && thrsh_cou != max_noise_samples){
                    thrsh_cou = 0;
                    if(!start) start = true;
                }

                if(abs(samp) < THRESHOLD && start){
                    thrsh_cou++;
                    delta = win_len-1 - i%win_len;
                    if(delta == 1){
                        /* проверяем средним скользящим окном есть ли наличие сигнала
                         * в выведенных отсчетах, потому что мог быть просто единичный выброс
                         * SWEEP_WINDOWS - максимум проходок
                         * meanwin_step - шаг по отсчетам
                         * mean_thrsh_lower - счетчик средних значений ниже порога */
                        const double meanwin_step = (i - start_win_offset + 1)/SWEEP_WINDOWS;
                        unsigned mean_thrsh_lower = 0;
                        for(unsigned j = 0; j < SWEEP_WINDOWS; j++){
                            unsigned sum = 0;
                            for(unsigned k = start_win_offset + j*meanwin_step; k < start_win_offset + (j+1)*meanwin_step; k++)
                                sum += abs(cuza.getSample(k));
                            if(sum/meanwin_step < THRESHOLD) mean_thrsh_lower++;
                        }
                        if(mean_thrsh_lower == SWEEP_WINDOWS){
                            thrsh_cou = 0;
                            delta = 0;
                            start = 0;
                        }

                    }
                }
                (*m_lineseries)->append(i/*(1.0/sampling_rate)*/, samp);
            }
        }
        else
            for(auto [i, start, thrsh_cou, delta, mean] =
                std::make_tuple(win_offset-1, false, 0, 0, 0.0);
                (win_offset-i < maxsamp) && (thrsh_cou >= max_noise_samples) ? delta < win_len-1 : 1; i--){
                qDebug() << "prev trying to get a sample at index" << i;
                qint16 samp = cuza.getSample(i);
                // если начало окна, то меняем сдвиг
                if(i % win_len == 0) win_offset = (--win_i)*win_len;
                (*m_lineseries)->append(i*(1.0/sampling_rate), samp);
                if(abs(samp) >= THRESHOLD && thrsh_cou != max_noise_samples){
                    thrsh_cou = 0;
                    if(!start) start = true;
                }
                if(abs(samp) < THRESHOLD && start){
                    thrsh_cou++;
                    delta = win_len-1 - i%win_len;
                    /*qDebug() << QString().sprintf("samp %3d    thrsh_i %3d    delta %3d", samp, i, delta);
                    qDebug() << QString().sprintf("i+delta %3d", i+delta);*/
                }
            }
    }
#endif
    m_chart->addSeries((*m_lineseries));
    m_chart->createDefaultAxes();
    m_chart->axisY()->setRange(-2048, 2048);
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
