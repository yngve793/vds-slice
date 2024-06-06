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

void unwrap(std::vector<double>& in, std::vector<double>& out) {

    int n = in.size();

    // Find angle
    std::vector<double> phase(n);
    for (int i = 0; i < phase.size(); i++) {
        phase[i] = std::arg(in[i]);
    }

    // Find difference signal[i+1] - signal[i]
    std::vector<double> difference(n - 1);
    for (int i = 0; i < difference.size(); i++) {
        difference[i] = in[i + 1] - in[i];
    }

    // Normalize in period and mod period. Finally return to original
    std::vector<double> difference_mod(n - 1);
    double two_pi = (double)(2 * M_PI);
    auto test = std::fmod(difference[13] + M_PI, 2 * M_PI);
    std::cout << test << std::endl;
    for (int i = 0; i < difference_mod.size(); i++) {
        auto t = std::fmod(difference[i] + M_PI, 2 * M_PI);
        if (t < 0) {
            t = t + 2 * M_PI;
        }
        difference_mod[i] = t - M_PI;
    }

    // Clean up zero values
    std::vector<double> ph_correct(n - 1);
    for (int i = 0; i < ph_correct.size(); i++) {
        ph_correct[i] = difference_mod[i] - difference[i];
    }

    std::vector<double> ph_correct_sum(n - 1);
    double ph_sum = 0;
    for (int i = 0; i < ph_correct_sum.size(); i++) {
        ph_sum += ph_correct[i];
        ph_correct_sum[i] = ph_sum;
    }

    // Not yet included
    // np.core.numeric.copyto(ph_correct, 0, where=abs(dd) < np.pi)

    // Apply corrections
    out[0] = in[0];
    // double ph_correct_sum = 0;
    for (int i = 1; i < n; i++) {
        // ph_correct_sum += ph_correct[i];
        out[i] = in[i] + ph_correct_sum[i - 1];
    }
}
