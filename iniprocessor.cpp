#include "iniprocessor.h"

INIProcessor::INIProcessor(QObject *parent) : QObject(parent){}

INIProcessor::~INIProcessor(){}

// чтение ini файла
bool INIProcessor::read(const QString& filename){
    Q_ASSERT(filename.contains(".ini", Qt::CaseInsensitive));
    if(QFileInfo(filename).exists()){
        QSettings ini(filename, QSettings::IniFormat);
        if(/*validate(ini) && */ini.status() == QSettings::NoError){

            auto readFloat = [](const QVariant& var)->float{
                // если целая и дробная части разделены запятой,
                // то value возвращает QVariant с QStringList
                if(!QString(var.typeName()).compare("QStringList"))
                    return var.toStringList().join(".").toFloat();
                else var.toFloat();
            };

            cuza.setFreq(readFloat(ini.value("MAIN/freq", 1812.5)));
            cuza.setIfFreq(readFloat(ini.value("MAIN/Fpch", 262.5)));
            cuza.setFd(readFloat(ini.value("MAIN/discrFreq", 350)));
            cuza.setVersion(ini.value("MAIN/version", 0).toInt());
            if(cuza.getVersion() == 3)
                cuza.setFreqInv((cuza.getFreq() < 562.5)
                     || (cuza.getFreq() > 3932.5) ? -1 : 1);
            else cuza.setFreqInv(ini.value("MAIN/freq_inv", 1).toInt());
            cuza.setMaxSamples(ini.value("MAIN/MAX_SAMPLES", 1024 * 1024 * 128).toUInt());
            cuza.setBuffLength(ini.value("MAIN/bufLength", 1024 * 1024 * 128 * 2).toUInt());
            if(ini.value("MAIN/N_FFT").toUInt())
                cuza.setNFFT(ini.value("MAIN/n_fft").toUInt());
            else
                cuza.setNFFT(ini.value("MAIN/NFFT", 8).toUInt());
            // Размер сэмпла в байтах (2)
            cuza.setSampWidth(ini.value("MAIN/sample_width", 2).toUInt());
            // Размер сэмпла в битах (12)
            cuza.setSampBitWidth(ini.value("MAIN/sample_bit_width", 12).toUInt());
            // Величина децимации (2)
            cuza.setDecimationRatio(ini.value("MAIN/decimation_ratio", 2).toUInt());
            cuza.setSampWinLen(2*cuza.getDecimationRatio()*(1<<cuza.getNFFT()));
            cuza.setPorog(ini.value("MAIN/porog", 20).toDouble());
            cuza.setPorogFFT(ini.value("MAIN/POROG_FFT", 0).toDouble());
            cuza.setSyncStart(ini.value("TIME/Sync_StartL", 0).toInt() |
                              ini.value("TIME/Sync_StartH", 0).toInt() << 32);
            cuza.setTimeStart(ini.value("TIME/Timer_StartL", 0).toInt() |
                              ini.value("TIME/Timer_StartH", 0).toInt() << 32);
            cuza.setMaxVisSamples(ini.value("MAIN/MAX_VIS_SAMPLES", 65536).toUInt());
            cuza.setMaxTimeSamples(ini.value("MAIN/MAX_TIME_SAMPLES", 65536).toInt());
            cuza.setShowRF(ini.value("MAIN/showRF", 1).toBool());
            cuza.setShowVideo(ini.value("MAIN/showVideo", 1).toBool());
            cuza.setShowPhase(ini.value("MAIN/showPhase", 1).toBool());
            cuza.setShowPIPOnly(ini.value("MAIN/showPhaseInPulseOnly", 0).toBool());
            cuza.setShowFreq(ini.value("MAIN/showFreq", 1).toBool());
            cuza.setShowFIPOnly(ini.value("MAIN/showFreqInPulseOnly", 0).toBool());
            cuza.setShowSpectrum(ini.value("MAIN/showSpectrum", 0).toBool());
            cuza.setSpectrumType(ini.value("MAIN/showSpectrumType", 0).toUInt());
            cuza.setSpectrumWindow(ini.value("MAIN/showSpectrumWindow", 0).toUInt());
            cuza.setSpectrumPoints(ini.value("MAIN/spectrumPoints", 1024).toUInt());
            cuza.setShowCoarseFreq(ini.value("MAIN/showCoarseFreq", 1).toBool());
            cuza.setShowCoarseVideo(ini.value("MAIN/showCoarseVideo", 1).toBool());
            cuza.setLinCorr(ini.value("MAIN/linCorr", 0).toDouble());
            cuza.setFFTScale(ini.value("MAIN/FFTScale", 0).toInt());
            cuza.setPhiCorr(ini.value("MAIN/phicorr", 0).toDouble());
            cuza.setSpecrogramPoints(ini.value("MAIN/spectrogramPoints", 32).toUInt());
            cuza.setFreqShift(ini.value("MAIN/freqShift", 0).toDouble());
            cuza.setHidePoints(ini.value("MAIN/HidePoints", 0).toUInt());
            cuza.setFineMixer(ini.value("MAIN/FineMixer", 0).toBool());
            cuza.setSoftLimiter(ini.value("MAIN/SoftLimiter", 1).toBool());
            cuza.setSpectrogramPerekr(ini.value("MAIN/spectrogram_perekr", 1).toUInt());
            cuza.setFreqSM(ini.value("MAIN/FREQ_SM", 1).toBool());
            cuza.setFreqSMRatio(ini.value("MAIN/FREQ_SM_RATIO", 16).toUInt());
            cuza.setCompMode(0);
            cuza.setFreqOffset(0);
            return true;
        }
    } else error(filename + " отсутсвует");
    return false;
}

// проверка ключей
// deprecated
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
