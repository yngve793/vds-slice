#include <complex>
#define EIGEN_FFTW_DEFAULT
#include <eigen3/unsupported/Eigen/FFT>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "transforms.hpp"

Eigen::FFT<double> eigen_fft;
void fft(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data) {

    eigen_fft.fwd(out_data, in_data);
}

void hilbert_transform(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data) {

    int n = in_data.size();
    int n_half = (int)n / 2;
    std::complex<double> two = {2, 0};
    std::vector<std::complex<double>> h(n, 0.0);
    h[0] = {1, 0};
    std::transform(h.begin() + 1, h.begin() + n_half, h.begin() + 1, [&two](std::complex<double> v) { return two; });
    if (n % 2 == 0) {
        h[n_half] = {1, 0};
    } else {
        h[n_half] = {2, 0};
    }

    fft(in_data, out_data);
    for (int i = 0; i < n; i++) {
        out_data[i] = out_data[i] * h[i];
    }

    ifft(out_data, out_data);
}

void ifft(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data) {

    eigen_fft.inv(out_data, in_data);
}
