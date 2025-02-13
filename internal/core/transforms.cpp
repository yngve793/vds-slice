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

void unwrap(std::vector<double>& in_data, std::vector<double>& out_data) {

    int n = in_data.size();

    // Find angle
    std::vector<double> phase(n);
    for (int i = 0; i < phase.size(); i++) {
        phase[i] = std::arg(in_data[i]);
    }

    // Find difference signal[i+1] - signal[i]
    std::vector<double> difference(n - 1);
    for (int i = 0; i < difference.size(); i++) {
        difference[i] = in_data[i + 1] - in_data[i];
    }

    // Normalize in period and mod period. Finally return to original
    std::vector<double> difference_mod(n - 1);
    double two_pi = (double)(2 * M_PI);
    auto test = std::fmod(difference[13] + M_PI, 2 * M_PI);
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

    // Apply corrections
    out_data[0] = in_data[0];
    for (int i = 1; i < n; i++) {
        out_data[i] = in_data[i] + ph_correct_sum[i - 1];
    }
}
