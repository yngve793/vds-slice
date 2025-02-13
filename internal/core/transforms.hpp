#ifndef TRANSFORMS_HPP
#define TRANSFORMS_HPP

#define EIGEN_FFTW_DEFAULT

#include <complex>
#include <vector>

void envelope(std::vector<std::complex<double>>& in_data, std::vector<double>& out_data);
void fft(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data);
void hilbert_transform(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data);
void ifft(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data);
void instantaneous_bandwidth(std::vector<std::complex<double>>& in_data, std::vector<double>& out_data);
void instantaneous_frequency(std::vector<std::complex<double>>& in_data, std::vector<double>& out_data);
void instantaneous_sweetness(std::vector<std::complex<double>>& in_data, std::vector<double>& out_data);
void phase(std::vector<std::complex<double>>& in_data, std::vector<double>& out_data);
void time_derivative(std::vector<double>& in_data, std::vector<double>& out_data);
void unwrap(std::vector<double>& in_data, std::vector<double>& out_data);

#endif /* TRANSFORMS_HPP */
