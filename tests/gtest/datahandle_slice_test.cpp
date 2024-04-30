#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>

#include "cppapi.hpp"
#include "datahandle.hpp"
#include "regularsurface.hpp"
#include "subvolume.hpp"

namespace {

/// Each value in the test files has the positional information encoded in little-endian.
/// Byte 0: Sample position
/// Byte 1: CrossLine position
/// Byte 2: InLine position
const std::string REGULAR_DATA = "file://regular_8x3_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x3_cube.vds";
const std::string SHIFT_8_BIG_DATA = "file://big_shift_8_32x3_cube.vds";
const std::string CREDENTIALS = "";

class DatahandleSliceTest : public ::testing::Test {

    void SetUp() override {
        iline_array = std::vector<int>(32);
        xline_array = std::vector<int>(32);
        sample_array = std::vector<int>(36);

        for (int i = 0; i < iline_array.size(); i++) {
            iline_array[i] = 3 + i * 3;
            xline_array[i] = 2 + i * 2;
        }

        for (int i = 0; i < sample_array.size(); i++) {
            sample_array[i] = 4 + i * 4;
        }

        single_datahandle = make_single_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str()
        );

        double_datahandle = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            &inplace_addition
        );

        double_reverse_datahandle = make_double_datahandle(
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            &inplace_addition
        );

        double_empty = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_8_BIG_DATA.c_str(),
            CREDENTIALS.c_str(),
            &inplace_addition
        );

        double_different_size = make_double_datahandle(
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_8_BIG_DATA.c_str(),
            CREDENTIALS.c_str(),
            &inplace_addition
        );
    }

    void TearDown() override {
        delete single_datahandle;
        delete double_datahandle;
        delete double_reverse_datahandle;
        delete double_empty;
        delete double_different_size;
    }

public:
    std::vector<int> iline_array;
    std::vector<int> xline_array;
    std::vector<int> sample_array;

    std::vector<Bound> slice_bounds;

    SingleDataHandle* single_datahandle;
    DoubleDataHandle* double_datahandle;
    DoubleDataHandle* double_reverse_datahandle;
    DoubleDataHandle* double_empty;
    DoubleDataHandle* double_different_size;

    /// @brief Check response slice towards expected slice
    /// @param response_data Data from request
    /// @param low Low limit on axis for expected data
    /// @param high High limit on axis for expected data
    /// @param factor multiplicative factor
    void check_slice(struct response response_data, int low[], int high[], float factor) {
        std::size_t nr_of_values = (std::size_t)(response_data.size / sizeof(float));

        EXPECT_EQ(nr_of_values, (high[0] - low[0]) * (high[1] - low[1]) * (high[2] - low[2]));

        int counter = 0;
        auto response_samples = (float*)response_data.data;
        for (int il = low[0]; il < high[0]; ++il) {
            for (int xl = low[1]; xl < high[1]; ++xl) {
                for (int s = low[2]; s < high[2]; s++) {
                    int value = std::lround(response_samples[counter] / factor);
                    int sample = value & 0xFF;
                    int xline = value & 0xFF00;
                    int iline = value & 0xFF0000;
                    iline = iline >> 16;
                    xline = xline >> 8;

                    EXPECT_EQ(iline, iline_array[il]);
                    EXPECT_EQ(xline, xline_array[xl]);
                    EXPECT_EQ(sample, sample_array[s]);
                    counter += 1;
                }
            }
        }
    }
};

TEST_F(DatahandleSliceTest, Slice_Axis_I_Single) {

    struct response response_data;
    cppapi::slice(
        *single_datahandle,
        Direction(axis_name::I),
        2,
        slice_bounds,
        &response_data
    );

    int low[3] = {2, 0, 0};
    int high[3] = {3, 8, 32};
    check_slice(response_data, low, high, 1);
}

TEST_F(DatahandleSliceTest, Slice_Axis_I_Double) {

    struct response response_data;
    cppapi::slice(
        *double_datahandle,
        Direction(axis_name::I),
        2,
        slice_bounds,
        &response_data
    );

    int low[3] = {6, 4, 4};
    int high[3] = {7, 8, 32};
    check_slice(response_data, low, high, 2);
}

TEST_F(DatahandleSliceTest, Slice_Axis_J_Single) {

    struct response response_data;
    cppapi::slice(
        *single_datahandle,
        Direction(axis_name::J),
        2,
        slice_bounds,
        &response_data
    );

    int low[3] = {0, 2, 0};
    int high[3] = {8, 3, 32};
    check_slice(response_data, low, high, 1);
}

TEST_F(DatahandleSliceTest, Slice_Axis_J_Double) {

    struct response response_data;
    cppapi::slice(
        *double_datahandle,
        Direction(axis_name::J),
        2,
        slice_bounds,
        &response_data
    );

    int low[3] = {4, 6, 4};
    int high[3] = {8, 7, 32};
    check_slice(response_data, low, high, 2);
}

TEST_F(DatahandleSliceTest, Slice_Axis_K_Single) {

    struct response response_data;
    cppapi::slice(
        *single_datahandle,
        Direction(axis_name::K),
        2,
        slice_bounds,
        &response_data
    );

    int low[3] = {0, 0, 2};
    int high[3] = {8, 8, 3};
    check_slice(response_data, low, high, 1);
}

TEST_F(DatahandleSliceTest, Slice_Axis_K_Double) {

    struct response response_data;
    cppapi::slice(
        *double_datahandle,
        Direction(axis_name::K),
        2,
        slice_bounds,
        &response_data
    );

    int low[3] = {4, 4, 6};
    int high[3] = {8, 8, 7};
    check_slice(response_data, low, high, 2);
}

TEST_F(DatahandleSliceTest, Slice_Axis_INLINE_Single) {

    struct response response_data;
    cppapi::slice(
        *single_datahandle,
        Direction(axis_name::INLINE),
        21,
        slice_bounds,
        &response_data
    );

    int low[3] = {6, 0, 0};
    int high[3] = {7, 8, 32};
    check_slice(response_data, low, high, 1);
}

TEST_F(DatahandleSliceTest, Slice_Axis_INLINE_Double) {

    struct response response_data;
    cppapi::slice(
        *double_datahandle,
        Direction(axis_name::INLINE),
        21,
        slice_bounds,
        &response_data
    );

    int low[3] = {6, 4, 4};
    int high[3] = {7, 8, 32};
    check_slice(response_data, low, high, 2);
}

TEST_F(DatahandleSliceTest, Slice_Axis_CROSSLINE_Single) {

    struct response response_data;
    cppapi::slice(
        *single_datahandle,
        Direction(axis_name::CROSSLINE),
        14,
        slice_bounds,
        &response_data
    );

    int low[3] = {0, 6, 0};
    int high[3] = {8, 7, 32};
    check_slice(response_data, low, high, 1);
}

TEST_F(DatahandleSliceTest, Slice_Axis_CROSSLINE_Double) {

    struct response response_data;
    cppapi::slice(
        *double_datahandle,
        Direction(axis_name::CROSSLINE),
        14,
        slice_bounds,
        &response_data
    );

    int low[3] = {4, 6, 4};
    int high[3] = {8, 7, 32};
    check_slice(response_data, low, high, 2);
}

TEST_F(DatahandleSliceTest, Slice_Axis_SAMPLE_Single) {

    struct response response_data;
    cppapi::slice(
        *single_datahandle,
        Direction(axis_name::SAMPLE),
        40,
        slice_bounds,
        &response_data
    );

    int low[3] = {0, 0, 9};
    int high[3] = {8, 8, 10};
    check_slice(response_data, low, high, 1);
}

TEST_F(DatahandleSliceTest, Slice_Axis_SAMPLE_Double) {

    struct response response_data;
    cppapi::slice(
        *double_datahandle,
        Direction(axis_name::SAMPLE),
        40,
        slice_bounds,
        &response_data
    );

    int low[3] = {4, 4, 9};
    int high[3] = {8, 8, 10};
    check_slice(response_data, low, high, 2);
}

TEST_F(DatahandleSliceTest, Slice_Axis_TIME_Single) {

    struct response response_data;
    cppapi::slice(
        *single_datahandle,
        Direction(axis_name::TIME),
        40,
        slice_bounds,
        &response_data
    );

    int low[3] = {0, 0, 9};
    int high[3] = {8, 8, 10};
    check_slice(response_data, low, high, 1);
}

TEST_F(DatahandleSliceTest, Slice_Axis_TIME_Double) {

    struct response response_data;
    cppapi::slice(
        *double_datahandle,
        Direction(axis_name::TIME),
        40,
        slice_bounds,
        &response_data
    );

    int low[3] = {4, 4, 9};
    int high[3] = {8, 8, 10};
    check_slice(response_data, low, high, 2);
}

TEST_F(DatahandleSliceTest, Slice_Axis_DEPTH_Single) {

    struct response response_data;

    EXPECT_THAT([&]() {
        cppapi::slice(
            *single_datahandle,
            Direction(axis_name::DEPTH),
            40,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Cannot fetch depth slice for VDS file with vertical axis unit: ms")));
}

TEST_F(DatahandleSliceTest, Slice_Axis_DEPTH_Double) {

    struct response response_data;

    EXPECT_THAT([&]() {
        cppapi::slice(
            *double_datahandle,
            Direction(axis_name::DEPTH),
            40,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Cannot fetch depth slice for VDS file with vertical axis unit: ms")));
}

TEST_F(DatahandleSliceTest, Slice_Axis_TIME_Invalid_Single) {

    Direction const direction(axis_name::TIME);
    struct response response_data;

    EXPECT_THAT([&]() {
        cppapi::slice(
            *single_datahandle,
            direction,
            0,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Invalid lineno: 0, valid range: [4.00:128.00:4.00]")));

    EXPECT_THAT([&]() {
        cppapi::slice(
            *single_datahandle,
            direction,
            132,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Invalid lineno: 132, valid range: [4.00:128.00:4.00]")));

    EXPECT_THAT([&]() {
        cppapi::slice(
            *single_datahandle,
            direction,
            21,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Invalid lineno: 21, valid range: [4.00:128.00:4.00]")));
}

TEST_F(DatahandleSliceTest, Slice_Axis_TIME_Invalid_Double) {

    Direction const direction(axis_name::TIME);
    struct response response_data;

    EXPECT_THAT([&]() {
        cppapi::slice(
            *double_datahandle,
            direction,
            16,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Invalid lineno: 16, valid range: [20.00:128.00:4.00]")));

    EXPECT_THAT([&]() {
        cppapi::slice(
            *double_datahandle,
            direction,
            132,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Invalid lineno: 132, valid range: [20.00:128.00:4.00]")));

    EXPECT_THAT([&]() {
        cppapi::slice(
            *double_datahandle,
            direction,
            21,
            slice_bounds,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Invalid lineno: 21, valid range: [20.00:128.00:4.00]")));
}

TEST_F(DatahandleSliceTest, Slice_Axis_TIME_Reverse_Double) {

    struct response response_data;
    cppapi::slice(
        *double_reverse_datahandle,
        Direction(axis_name::TIME),
        40,
        slice_bounds,
        &response_data
    );

    int low[3] = {4, 4, 9};
    int high[3] = {8, 8, 10};
    check_slice(response_data, low, high, 2);
}

} // namespace
