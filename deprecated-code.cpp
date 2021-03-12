#include <QtCore>
#include "dataprocessor.h"

// установление битов в number в обратном порядке
// для 12-битного числа
auto reversebits = [](quint16 number)->quint16{
    quint16 mask1_1 = 0b111111000000, mask1_2 = 0b000000111111,
            mask2_1 = 0b111000111000, mask2_2 = 0b000111000111,
            mask3_1 = 0b100100100100, mask3_2 = 0b001001001001, mask3_3 = 0b010010010010;
    number = (number & mask1_1) >> 6 | (number & mask1_2) << 6;
    number = (number & mask2_1) >> 3 | (number & mask2_2) << 3;
    number = (number & mask3_1) >> 2 | (number & mask3_2) << 2 | (number & mask3_3);
    return number;
};

// попытуки реализовать бпф
//complex_ptr DataProcessor::fft(std::vector<complex> &in, unsigned N)
//{
//   #define COULIE
//   qDebug() << "here fft";
//#ifndef COULIE
//   complex* fft_ptr = new complex[N/2];
//   for(unsigned i = 0; i < N/2; i++)
//       for(unsigned j = 0; j < N/2; j++){
//           double arg = 2*M_PI*j*i/(double)N;
//           fft_ptr[i] += complex(in[j*2]*cos(arg), -in[j*2]*sin(arg)) +
//                   complex(in[j*2+1]*cos(arg), -in[j*2+1]*sin(arg)) * exp(complex(0, -2*M_PI*i/(double)N));
//       }
//   return complex_ptr(fft_ptr);
//#else
//    fftN = N;
//    double LN = log2(N);
//    // если логарифм не целое число
//    // дополняем in нулями до ближайшего числа степени 2
//    // при преобразовании фурье все равно будем брать исходное кол-во отсчетов
//    if(floor(LN) != LN){
//        N = pow(2, int(LN)+1);
//        in.resize(N);
//        std::fill(in.begin()+fftN, in.end(), 0);
//    }
//    std::vector<complex*> p_in(N);
//    for(unsigned i = 0; i < N; i++) p_in[i] = &in[i];
//    if(!hilb){
//        complex_ptr fft_ptr = fft_inside(p_in, N);
//        complex* fft_res = new complex[fftN];
//        memcpy(fft_res, fft_ptr.get(), fftN);
//        return complex_ptr(fft_res);
//    } else return fft_inside(p_in, N);
//#endif
//}

//complex_ptr DataProcessor::fft_inside(const std::vector<complex*>& in, const unsigned N)
//{
//    // алгоритм Кули-Тьюки
//    static unsigned orig_N = 0;
//    if(!orig_N) orig_N = N;
//    if(N == 1)
//        // т.к. отсчет остался один, то агрумент экспоненты превращается в 0
//        // следовательно, можно просто вернуть этот отсчет
//        /*
//        double arg = 2*M_PI*0*0/(double)N;
//        fft_ptr[0] = in[0]*complex(cos(arg), -sin(arg));
//        */
//        return complex_ptr(in[0]);
//    else if (N < 1) return NULL;
//    complex_ptr fft_ptr     =    fft_inside([in, N]()->std::vector<complex*>{
//                                                std::vector<complex*> vec(N/2);
//                                                for(unsigned i = 0; i < N/2; i++) vec[i] = in[i*2];
//                                                return vec;
//                                            }(), N/2),
//                fft_ptr_2   =    fft_inside([in, N]()->std::vector<complex*>{
//                                                std::vector<complex*> vec(N/2);
//                                                for(unsigned i = 0; i < N/2; i++) vec[i] = in[i*2+1];
//                                                return vec;
//                                            }(), N/2);
//    // объединение
//    complex*    fft_res = new complex[N];
//    for(unsigned i = 0; i < N/2; i++){
//        fft_res[i] = fft_ptr.get()[i]+fft_ptr_2.get()[i]*exp(complex(0, -2*M_PI*i/(double)N));
//        fft_res[i+N/2] = fft_ptr.get()[i]-fft_ptr_2.get()[i]*exp(complex(0, -2*M_PI*i/(double)N));
//    }
//    if(N == orig_N) orig_N = 0;
//    return complex_ptr(fft_res);
//}

//complex_ptr DataProcessor::inv_fft(const complex_ptr spectrum, unsigned N)
//{
//    complex* fft_ptr = new complex[N];
//    std::vector<complex> spec_vec(N);
//    std::vector<complex*> p_spec_vec(N);
//    for(unsigned i = 0; i < N; i++){
//        spec_vec[i] = std::conj(spectrum.get()[i]);
//        p_spec_vec[i] = &spec_vec[i];
//    }
//    // 090321 доделать дополнение нулями
//    if(!hilb){
//        fftN = N;
//        double LN = log2(N);
//        // если логарифм не целое число
//        // дополняем in нулями до ближайшего числа степени 2
//        // при преобразовании фурье все равно будем брать исходное кол-во отсчетов
////        if(floor(LN) != LN){
////            N = pow(2, int(LN)+1);
////            in.resize(N);
////            std::fill(in.begin()+fftN, in.end(), 0);
////        }
//    }
//    complex_ptr spec_fft = fft_inside(p_spec_vec, N);
//    for(unsigned i = 0; i < N; i++) spec_fft.get()[i] = std::conj(spec_fft.get()[i]);
//    if(hilb){
//        fft_ptr = 0;
//    }
//    for(unsigned i = 0; i < N/2; i++)
//        for(unsigned j = 0; j < N/2; j++){
//            double arg = 2*M_PI*j*i/(double)N;
//            fft_ptr[i*2] += spectrum.get()[j]*complex(cos(arg)/(N/2), sin(arg)/(N/2));
//            fft_ptr[i*2+1] += spectrum.get()[j]*complex(cos(arg)/(N/2), sin(arg)/(N/2))*exp(complex(0, 2*M_PI*i/(double)N));
//        }
//    return complex_ptr(fft_ptr);
//}
