#include <complex>
#define EIGEN_FFTW_DEFAULT
#include <eigen3/unsupported/Eigen/FFT>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "transforms.hpp"

void fft(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out) {

    Eigen::FFT<double> fft;
    fft.fwd(out, in);
}

void ifft(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out) {

    Eigen::FFT<double> fft;
    fft.inv(out, in);
}

void hilbert_transform(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out) {

    int n = in.size();
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

    fft(in, out);
    for (int i = 0; i < n; i++) {
        out[i] = out[i] * h[i];
    }

    ifft(out, out);
}
