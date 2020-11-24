#include "iniprocessor.h"

INIProcessor::INIProcessor(QObject *parent) : QObject(parent){}

INIProcessor::~INIProcessor(){}

// чтение ini файла
void INIProcessor::read(const QString& filename){
    Q_ASSERT(filename.contains(".ini", Qt::CaseInsensitive));
    if(QFileInfo(filename).exists()){
        QSettings ini(filename, QSettings::IniFormat);
        if(validate(ini) && ini.status() == QSettings::NoError){

            auto readFloat = [](const QVariant& var)->float{
                // если целая и дробная части разделены запятой,
                // то value возвращает QVariant с QStringList
                if(!QString(var.typeName()).compare("QStringList"))
                    return var.toStringList().join(".").toFloat();
                else var.toFloat();
            };

            cuza.setFreq(readFloat(ini.value("MAIN/freq")));
            cuza.setIfFreq(readFloat(ini.value("MAIN/Fpch")));
            cuza.setFd(readFloat(ini.value("MAIN/discrFreq")));
            // 241120
            cuza.s
        }
    } else error(filename + " отсутсвует");
}

// проверка ключей
bool INIProcessor::validate(const QSettings& ini){
    QStringList keys = ini.allKeys();
    for(QString str : keys){
        if(str.contains("coefs", Qt::CaseInsensitive)
                || !str.compare("MAIN/freq_inv")
                || str.contains("firLen", Qt::CaseInsensitive)
                || str.contains("firDesc", Qt::CaseInsensitive))
            keys.removeOne(str);
    }
    QStringList legitKeys;
    legitKeys   << "MAIN/bufLength" << "MAIN/INPUT_ATT" << "MAIN/porog"
                << "MAIN/PCH_ATT" << "MAIN/firCount" << "MAIN/firIndex"
                << "MAIN/discrFreq" << "MAIN/ackMode" << "MAIN/ackWidth"
                << "MAIN/freq" << "MAIN/Fpch" << "MAIN/wObnaruz" << "MAIN/showRF"
                << "MAIN/showVideo" << "MAIN/showPhase" <<"MAIN/showPhaseInPulseOnly"
                << "MAIN/showFreq" << "MAIN/showFreqInPulseOnly" << "MAIN/showSpectrum"
                << "MAIN/showSpectrumType" << "MAIN/unwrapPhaseSpectrum" << "MAIN/showSpectrumWindow"
                << "MAIN/spectrumPoints" << "MAIN/linCorr" << "MAIN/MAX_SAMPLES"
                << "MAIN/MAX_VIS_SAMPLES" << "MAIN/graphDecim" << "MAIN/USBblockLen"
                << "MAIN/showCoarseFreq" << "MAIN/showCoarseVideo" << "MAIN/FFTScale"
                << "MAIN/phicorr" << "MAIN/comp_mode" << "MAIN/spectrogramPoints"
                << "MAIN/freqShift" << "MAIN/timeShift" << "MAIN/amplRatio"
                << "MAIN/freqShiftStep" << "MAIN/timeShiftStep" << "MAIN/amplRatioStep"
                << "MAIN/POROG_FFT" << "MAIN/pulseCut" << "MAIN/HidePoints" << "MAIN/MAX_TIME_SAMPLES"
                << "MAIN/FineMixer" << "MAIN/SoftLimiter" << "MAIN/NFFT" << "MAIN/FREQ_SM"
                << "MAIN/FREQ_SM_RATIO" << "MAIN/PAN_cont" << "MAIN/FileName" << "MAIN/version"
                << "MAIN/comments" << "TIME/Timer_StartH" << "TIME/Timer_StartL"
                << "TIME/Sync_StartH" << "TIME/Sync_StartL" << "WORK/spectrogram_perekr";
    QSet<QString> lack = legitKeys.toSet().subtract(keys.toSet());
    QSet<QString> surplus = keys.toSet().subtract(legitKeys.toSet());
    bool err_b = false, warn_b = false;
    QString err = ini.fileName()+"\n\n";
    if(lack.size()){
        err_b = true;
        err += "Отсутсвуют ключи:\n";
        foreach(QString str, lack)
            err += "\t" + str + "\n";
    }
    if(surplus.size()){
        warn_b = true;
        err += "Лишние ключи:\n";
        foreach(QString str, surplus)
            err += "\t" + str + "\n";
    }
    if(err_b) error(err);
    if(warn_b && !err_b) warning(err);
    return !err_b;
}

void INIProcessor::error(const QString &msg)
{
    Error errBox;
    errBox.ShowMessage(msg);
}

void INIProcessor::warning(const QString &msg)
{
    Error warnBox(ErrType::WARN);
    warnBox.ShowMessage(msg);
}
