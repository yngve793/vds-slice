#ifndef TRANSFORMS_HPP
#define TRANSFORMS_HPP

#define EIGEN_FFTW_DEFAULT

#include <complex>
#include <vector>

void fft(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data);
void hilbert_transform(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data);
void ifft(std::vector<std::complex<double>>& in_data, std::vector<std::complex<double>>& out_data);

#endif /* TRANSFORMS_HPP */
