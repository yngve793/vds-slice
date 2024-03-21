#ifndef CONVOLUTIONS_HPP
#define CONVOLUTIONS_HPP

#include <vector>
#include <complex>

void fft_power_2(const std::vector<std::complex<double>> data, std::vector<std::complex<double>>* Y);

void ifft_power_2(const std::vector<std::complex<double>> data, std::vector<std::complex<double>>* Y);

void fft(const std::vector<std::complex<double>>& data, std::vector<std::complex<double>>& result);

void ifft(const std::vector<std::complex<double>>& data, std::vector<std::complex<double>>& result);

void hilbert_transform(const std::vector<std::complex<double>>& data, std::vector<std::complex<double>>& result);





#endif /* CONVOLUTIONS_HPP */
