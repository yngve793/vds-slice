#ifndef TRANSFORMS_HPP
#define TRANSFORMS_HPP

#define EIGEN_FFTW_DEFAULT

#include <complex>
#include <vector>

void fft(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out);

void ifft(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out);

void hilbert_transform(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out);

void unwrap(std::vector<double>& in, std::vector<double>& out);

#endif /* TRANSFORMS_HPP */
