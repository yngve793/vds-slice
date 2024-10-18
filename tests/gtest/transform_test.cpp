#include <complex>
#include <iostream>
#include <sstream>

#include "transforms.hpp"

#include "nlohmann/json.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <fstream>

namespace {

const std::string TRANSFORM_EXPECTED = "transform_expected.json";
const std::string PETREL_EXPECTED = "petrel_attribute_data.json";
class transformTest : public ::testing::Test {

    void SetUp() override {
        std::ifstream f(TRANSFORM_EXPECTED.c_str());
        transform_expected = nlohmann::json::parse(f);
        std::vector<double> input_data_143 = transform_expected["input_data_143"].template get<std::vector<double>>();
        std::vector<double> input_data_144 = transform_expected["input_data_144"].template get<std::vector<double>>();
        frequency_1 = transform_expected["f1"].template get<std::int32_t>();
        frequency_2 = transform_expected["f2"].template get<std::int32_t>();

        // input_cdata.reserve(input_data.size());
        input_cdata_143 = std::vector<std::complex<double>>(input_data_143.size());
        for (int i = 0; i < input_data_143.size(); i++) {
            input_cdata_143[i].real(input_data_143[i]);
        }

        input_cdata_144 = std::vector<std::complex<double>>(input_data_144.size());
        for (int i = 0; i < input_data_144.size(); i++) {
            input_cdata_144[i].real(input_data_144[i]);
        }

        std::ifstream f_p(PETREL_EXPECTED.c_str());
        petrel_expected = nlohmann::json::parse(f_p);
        // std::vector<double> petrel_bw = petrel_expected["BW"].template get<std::vector<double>>();
        // std::vector<double> petrel_env = petrel_expected["Env"].template get<std::vector<double>>();
        // std::vector<double> petrel_freq = petrel_expected["Freq"].template get<std::vector<double>>();
        std::vector<double> petrel_phase = petrel_expected["Phase"].template get<std::vector<double>>();
        // std::vector<double> petrel_sweet = petrel_expected["Sweet"].template get<std::vector<double>>();
        std::vector<double> petrel_raw = petrel_expected["raw"].template get<std::vector<double>>();
    }

    void TearDown() override {
    }

public:
    nlohmann::json transform_expected;
    nlohmann::json petrel_expected;
    std::vector<std::complex<double>> input_cdata_143;
    std::vector<std::complex<double>> input_cdata_144;

    std::int32_t n;
    std::int32_t frequency_1;
    std::int32_t frequency_2;
};

TEST_F(transformTest, transform_odd_fft_test) {

    std::vector<double> expected_fft_result_real = transform_expected["fft_result_real_143"].template get<std::vector<double>>();
    std::vector<double> expected_fft_result_imag = transform_expected["fft_result_imag_143"].template get<std::vector<double>>();

    std::vector<std::complex<double>> fft_result(input_cdata_143.size());
    fft(input_cdata_143, fft_result);

    EXPECT_EQ(fft_result.size(), 143);

    for (int i = 0; i < fft_result.size(); i++) {
        EXPECT_NEAR(fft_result[i].real(), expected_fft_result_real[i], 2e-14) << "Unexpected real value at index " << i;
        EXPECT_NEAR(fft_result[i].imag(), expected_fft_result_imag[i], 2e-14) << "Unexpected imag value at index " << i;

        if (i != frequency_1 && i != frequency_2 && i != fft_result.size() - frequency_2 && i != fft_result.size() - frequency_1) {
            EXPECT_NEAR(std::real(fft_result[i]), 0, 2e-13) << "Unexpected real value at index " << i;
            EXPECT_NEAR(std::imag(fft_result[i]), 0, 2e-13) << "Unexpected imag value at index " << i;
        } else {
            if (i == frequency_1) {
                EXPECT_NEAR(std::real(fft_result[i]), 0, 2e-12) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), -71.5, 2e-14) << "Unexpected imag value at index " << i;
            } else if (i == frequency_2) {
                EXPECT_NEAR(std::real(fft_result[i]), 71.5, 2e-13) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), 0, 2e-13) << "Unexpected imag value at index " << i;
            } else if (i == fft_result.size() - frequency_1) {
                EXPECT_NEAR(std::real(fft_result[i]), 0, 2e-12) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), 71.5, 2e-14) << "Unexpected imag value at index " << i;
            } else if (i == frequency_2 || i == fft_result.size() - frequency_2) {
                EXPECT_NEAR(std::real(fft_result[i]), 71.5, 2e-13) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), 0, 2e-13) << "Unexpected imag value at index " << i;
            }
        }
    }
}

TEST_F(transformTest, transform_even_fft_test) {

    std::vector<double> expected_fft_result_real = transform_expected["fft_result_real_144"].template get<std::vector<double>>();
    std::vector<double> expected_fft_result_imag = transform_expected["fft_result_imag_144"].template get<std::vector<double>>();

    std::vector<std::complex<double>> fft_result(input_cdata_144.size());
    fft(input_cdata_144, fft_result);

    EXPECT_EQ(fft_result.size(), 144);

    for (int i = 0; i < fft_result.size(); i++) {
        EXPECT_NEAR(fft_result[i].real(), expected_fft_result_real[i], 2e-14) << "Unexpected real value at index " << i;
        EXPECT_NEAR(fft_result[i].imag(), expected_fft_result_imag[i], 2e-14) << "Unexpected imag value at index " << i;

        if (i != frequency_1 && i != frequency_2 && i != fft_result.size() - frequency_2 && i != fft_result.size() - frequency_1) {
            EXPECT_NEAR(std::real(fft_result[i]), 0, 2e-13) << "Unexpected real value at index " << i;
            EXPECT_NEAR(std::imag(fft_result[i]), 0, 2e-13) << "Unexpected imag value at index " << i;
        } else {
            if (i == frequency_1) {
                EXPECT_NEAR(std::real(fft_result[i]), 0, 2e-12) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), -72.0, 2e-14) << "Unexpected imag value at index " << i;
            } else if (i == frequency_2) {
                EXPECT_NEAR(std::real(fft_result[i]), 72.0, 2e-14) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), 0, 2e-12) << "Unexpected imag value at index " << i;
            } else if (i == fft_result.size() - frequency_1) {
                EXPECT_NEAR(std::real(fft_result[i]), 0, 2e-12) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), 72.0, 2e-14) << "Unexpected imag value at index " << i;
            } else if (i == frequency_2 || i == fft_result.size() - frequency_2) {
                EXPECT_NEAR(std::real(fft_result[i]), 72.0, 2e-14) << "Unexpected real value at index " << i;
                EXPECT_NEAR(std::imag(fft_result[i]), 0, 2e-12) << "Unexpected imag value at index " << i;
            }
        }
    }
}

TEST_F(transformTest, transform_ifft_odd_test) {

    std::vector<std::complex<double>> fft_result(input_cdata_143.size());
    fft(input_cdata_143, fft_result);

    // reverse
    std::vector<std::complex<double>> input_cdata_2(input_cdata_143.size());
    ifft(fft_result, input_cdata_2);

    for (int i = 0; i < input_cdata_2.size(); i++) {
        EXPECT_NEAR(std::real(input_cdata_2[i]), std::real(input_cdata_143[i]), 1e-15);
        EXPECT_NEAR(std::imag(input_cdata_2[i]), std::imag(input_cdata_143[i]), 1e-15);
    }
}

TEST_F(transformTest, transform_ifft_even_test) {

    std::vector<std::complex<double>> fft_result(input_cdata_144.size());
    fft(input_cdata_144, fft_result);

    // reverse
    std::vector<std::complex<double>> input_cdata_2(input_cdata_144.size());
    ifft(fft_result, input_cdata_2);

    for (int i = 0; i < input_cdata_2.size(); i++) {
        EXPECT_NEAR(std::real(input_cdata_2[i]), std::real(input_cdata_144[i]), 2e-15);
        EXPECT_NEAR(std::imag(input_cdata_2[i]), std::imag(input_cdata_144[i]), 2e-15);
    }
}

TEST_F(transformTest, transform_odd_Hilbert_test) {

    std::vector<double> expected_hilbert_real = transform_expected["hilbert_result_real_143"].template get<std::vector<double>>();
    std::vector<double> expected_hilbert_imag = transform_expected["hilbert_result_imag_143"].template get<std::vector<double>>();

    std::vector<std::complex<double>> hilbert_result(input_cdata_143.size());
    hilbert_transform(input_cdata_143, hilbert_result);

    for (int i = 0; i < hilbert_result.size(); i++) {
        EXPECT_NEAR(std::real(hilbert_result[i]), expected_hilbert_real[i], 2e-15) << "Unexpected real value at index " << i;
        EXPECT_NEAR(std::imag(hilbert_result[i]), expected_hilbert_imag[i], 2e-15) << "Unexpected real value at index " << i;
    }
}

TEST_F(transformTest, transform_even_Hilbert_test) {

    std::vector<double> expected_hilbert_real = transform_expected["hilbert_result_real_144"].template get<std::vector<double>>();
    std::vector<double> expected_hilbert_imag = transform_expected["hilbert_result_imag_144"].template get<std::vector<double>>();

    std::vector<std::complex<double>> hilbert_result(input_cdata_144.size());
    hilbert_transform(input_cdata_144, hilbert_result);

    for (int i = 0; i < hilbert_result.size(); i++) {
        EXPECT_NEAR(std::real(hilbert_result[i]), expected_hilbert_real[i], 2e-15) << "Unexpected real value at index " << i;
        EXPECT_NEAR(std::imag(hilbert_result[i]), expected_hilbert_imag[i], 2e-15) << "Unexpected real value at index " << i;
    }
}

/// @brief The hilbert_transform function handles odd and even length input differently
TEST_F(transformTest, Hilbert_unit_test_even) {

    std::vector<std::complex<double>> s(100, 0.0f);
    for (int i = 0; i < s.size(); i++) {
        s[i] = std::sin(i * 2 * M_PI * 5 / s.size());
    }

    std::vector<std::complex<double>> res(s.size(), 0.0f);
    hilbert_transform(s, res);

    for (int i = 5; i < s.size(); i++) {
        EXPECT_NEAR(std::real(s[i - 5]), std::imag(res[i]), 1e-14);
    }
}

/// @brief The hilbert_transform function handles odd and even length input differently
TEST_F(transformTest, Unwrap_test) {

    std::vector<double> input_unwrap = transform_expected["input_unwrap"].template get<std::vector<double>>();
    std::vector<double> expected_output_unwrap = transform_expected["output_unwrap"].template get<std::vector<double>>();

    std::vector<double> output_unwrap(input_unwrap.size());
    unwrap(input_unwrap, output_unwrap);

    EXPECT_EQ(input_unwrap.size(), output_unwrap.size());

    for (int i = 0; i < output_unwrap.size(); i++) {
        auto value = output_unwrap[i] - expected_output_unwrap[i];
        if (value != 0) {
            std::cout << i << " " << value << std::endl;
        }
        EXPECT_NEAR(output_unwrap[i], expected_output_unwrap[i], 2e-15) << "Unexpected phase value at index " << i;
    }
}

TEST_F(transformTest, transform_envelope_test) {

    std::vector<double> petrel_env = petrel_expected["Env"].template get<std::vector<double>>();
    std::vector<double> petrel_raw = petrel_expected["raw"].template get<std::vector<double>>();

    std::vector<std::complex<double>> in_data(petrel_raw.size());
    std::vector<double> res_data(petrel_raw.size());

    for (int i = 0; i < petrel_raw.size(); i++) {
        in_data[i].real(petrel_raw[i]);
    }

    envelope(in_data, res_data);

    for (int i = 0; i < res_data.size(); i++) {
        std::cout << i << " " << res_data[i] << " " << petrel_env[i] << std::endl;
        // EXPECT_NEAR(petrel_env[i], res_data[i]) << "Unexpected real value at index " << i;
    }
}


} // namespace
