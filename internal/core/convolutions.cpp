#include <iostream>

#include "convolutions.hpp"

#define PFFFT_ENABLE_DOUBLE
#include "pffft/pffft.hpp"

void fft_power_2(const std::vector<std::complex<double>> data, std::vector<std::complex<double>>* Y){
    
    int transformLen = data.size();
    using FFT_T = pffft::Fft<double>;
    if (transformLen < FFT_T::minFFtsize()) {
        std::cerr << "Error: minimum FFT transformation length is " << FFT_T::minFFtsize() << std::endl;
    }

    // instantiate FFT and prepare transformation for length N
    pffft::Fft<std::complex<double>> fft{transformLen};

    // one more check
    if (!fft.isValid()) {
        std::cerr << "Error: transformation length " << transformLen << " is not decomposable into small prime factors. "
                  << "Next valid transform size is: " << FFT_T::nearestTransformSize(transformLen)
                  << "; next power of 2 is: " << FFT_T::nextPowerOfTwo(transformLen) << std::endl;
    }
    fft.forward(data.data(), Y->data());
}

void ifft_power_2(const std::vector<std::complex<double>> data, std::vector<std::complex<double>>* Y){
    
    int transformLen = data.size();
    using FFT_T = pffft::Fft<double>;
    if (transformLen < FFT_T::minFFtsize()) {
        std::cerr << "Error: minimum FFT transformation length is " << FFT_T::minFFtsize() << std::endl;
    }

    // instantiate FFT and prepare transformation for length N
    pffft::Fft<std::complex<double>> fft{transformLen};

    // one more check
    if (!fft.isValid()) {
        std::cerr << "Error: transformation length " << transformLen << " is not decomposable into small prime factors. "
                  << "Next valid transform size is: " << FFT_T::nearestTransformSize(transformLen)
                  << "; next power of 2 is: " << FFT_T::nextPowerOfTwo(transformLen) << std::endl;
    }
    fft.inverse(data.data(), Y->data());
}

void fft(const std::vector<std::complex<double>>& data, std::vector<std::complex<double>>& result){

    // Bluestein's algorithm  
    // https://en.wikipedia.org/wiki/Chirp_Z-transform#Bluestein%27s_algorithm
    int n = data.size();
    int n2 = int(std::pow(2, std::ceil(std::log2(2*n-1)))); // Next power if two 

    // Compute the chirp function 
    std::complex<double> w = std::exp(-2.0*(std::complex<double>{0, 1.0f})* M_PI / (double)n);
    std::vector<std::complex<double>> chirp(2*n+1);
    for (int i = 0; i < chirp.size(); i++){
        double t = 1-n+i;
        chirp[i] = std::pow(w, t*t/2); 
    }

    std::vector<std::complex<double>> xp(n2, 0.0);
    std::vector<std::complex<double>> fft_xp(n2);
    for (int i = 0; i < data.size(); i++){
        xp[i] = data[i] * std::pow(1.0, (double)(-i))  * chirp[n-1+i];
    }
    fft_power_2(xp, &fft_xp);

    std::vector<std::complex<double>> ichirpp(n2, 0.0);
    std::vector<std::complex<double>> fft_ichirpp(n2);
    for (int i = 0; i < 2*n-1; i++){
        ichirpp[i] = 1.0 / chirp[i];
    }
    fft_power_2(ichirpp, &fft_ichirpp);

    // Multiply the two fft and do the ifft
    for (int i = 0; i < n2; i++){
        fft_xp[i] *= fft_ichirpp[i];
    }
    std::vector<std::complex<double>> res(n2);
    ifft_power_2(fft_xp, &res);

    // Extract data
    for (int i = 0; i < n; i++){
        result[i] = (res[n-1+i] * chirp[n-1+i]) / (double)n2;
    }

    // std::cout << std::endl;
    // std::cout << "IFFT part 1 Inside:" <<std::endl;
    // for (int i = 0; i < result.size(); i++){
    //     std::cout << i << " " << data[i] << "  " << result[i] << std::endl;
    // }
    // std::cout << std::endl;

}

void ifft(const std::vector<std::complex<double>>& data, std::vector<std::complex<double>>& result){

    std::vector<std::complex<double>> data_conj(data.size());
    for (int i = 0; i < data_conj.size(); i++){
        data_conj[i] = std::conj(data[i]);
    }

    fft(data_conj, result);

    // std::cout << std::endl;
    // std::cout << "IFFT part 1:" <<std::endl;
    // for (int i = 0; i < result.size(); i++){
    //     std::cout << i << " " << data_conj[i] << "  " << result[i] << std::endl;
    // }
    // std::cout << std::endl;



    for (int i = 0; i < result.size(); i++){
        result[i] = std::conj(result[i]) / (double)data.size();
    }
}

void hilbert_transform(const std::vector<std::complex<double>>& data, std::vector<std::complex<double>>& result){
    
    int n = data.size();
    int n_half = (int)n/2;
    fft(data, result);
    std::vector<std::complex<double>> h(n, 0.0);
    if (n % 2 ==0){
        h[0] = {1,0};
        h[n_half] = {1,0};
        for (int i = 1; i < n_half; i++){
            h[i] = {2,0};
        }
    } else {
        h[0] = {1,0};
        for (int i = 1; i < (int)(n+1)/2; i++){
            h[i] = {2,0};
        }
    }

    for (int i = 0; i < n; i++){
        result[i] = result[i] * h[i];
    }

    ifft(result, result);
}