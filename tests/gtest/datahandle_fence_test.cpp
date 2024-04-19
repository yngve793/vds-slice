#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>

#include "cppapi.hpp"
#include "datahandle.hpp"
#include "regularsurface.hpp"
#include "subvolume.hpp"

namespace {
const std::string REGULAR_DATA = "file://regular_8x3_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x3_cube.vds";
const std::string SHIFT_8_BIG_DATA = "file://big_shift_8_32x3_cube.vds";
const std::string CREDENTIALS = "";

class DatahandleFenceTest : public ::testing::Test {

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

    SingleDataHandle* single_datahandle;
    DoubleDataHandle* double_datahandle;
    DoubleDataHandle* double_reverse_datahandle;
    DoubleDataHandle* double_empty;
    DoubleDataHandle* double_different_size;

    static constexpr float fill = -999.25;

    void check_fence(struct response response_data, std::vector<float> coordinates, int low[], int high[], float factor, bool fill_flag) {
        std::size_t nr_of_values = (std::size_t)(response_data.size / sizeof(float));
        std::size_t nr_of_traces = (std::size_t)(coordinates.size() / 2);

        EXPECT_EQ(nr_of_values, nr_of_traces * (high[2] - low[2]));

        int counter = 0;
        for (int t = 0; t < nr_of_traces; t++) {
            int ic = coordinates[2 * t];
            int xc = coordinates[(2 * t) + 1];
            for (int s = low[2]; s < high[2]; ++s) {
                float value = *(float*)&response_data.data[counter * sizeof(float)];
                if (fill_flag) {
                    int intValue = int((value / factor) + 0.5f);
                    int sample = intValue & 0xFF;
                    int xline = (intValue & 0xFF00) >> 8;
                    int iline = (intValue & 0xFF0000) >> 16;

                    EXPECT_EQ(iline, iline_array[ic]);
                    EXPECT_EQ(xline, xline_array[xc]);

                    EXPECT_EQ(sample, sample_array[s]);
                } else {
                    EXPECT_EQ(value, fill);
                }

                counter += 1;
            }
        }
    }
};

TEST_F(DatahandleFenceTest, Fence_INDEX_Single) {

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7};

    cppapi::fence(
        *single_datahandle,
        coordinate_system::INDEX,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {0, 0, 0};
    int high[3] = {8, 8, 32};
    check_fence(response_data, coordinates, low, high, 1, true);
}

TEST_F(DatahandleFenceTest, Fence_INDEX_Double) {

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const std::vector<float> check_coordinates{4, 4, 5, 5, 6, 6, 7, 7};

    cppapi::fence(
        *double_datahandle,
        coordinate_system::INDEX,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {4, 4, 4};
    int high[3] = {8, 8, 32};
    check_fence(response_data, check_coordinates, low, high, 2, true);
}

TEST_F(DatahandleFenceTest, Fence_INDEX_Fill_Single) {

    struct response response_data;
    const std::vector<float> coordinates{-1, -1, 8, 8};

    cppapi::fence(
        *single_datahandle,
        coordinate_system::INDEX,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        &fill,
        &response_data
    );

    int low[3] = {0, 0, 0};
    int high[3] = {2, 2, 32};
    check_fence(response_data, coordinates, low, high, 1, false);
}

TEST_F(DatahandleFenceTest, Fence_INDEX_Fill_Double) {

    struct response response_data;
    const std::vector<float> coordinates{-1, -1, 4, 4};

    cppapi::fence(
        *double_datahandle,
        coordinate_system::INDEX,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        &fill,
        &response_data
    );

    int low[3] = {0, 0, 4};
    int high[3] = {2, 2, 32};
    check_fence(response_data, coordinates, low, high, 1, false);
}

TEST_F(DatahandleFenceTest, Fence_INDEX_out_Of_Bounds_Single) {

    struct response response_data;
    const std::vector<float> coordinates{8, 8};

    EXPECT_THAT([&]() {
        cppapi::fence(
            *single_datahandle,
            coordinate_system::INDEX,
            coordinates.data(),
            int(coordinates.size() / 2),
            NEAREST,
            nullptr,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate (8.000000,8.000000) is out of boundaries in dimension 0.")));
}

TEST_F(DatahandleFenceTest, Fence_INDEX_out_Of_Bounds_Double) {

    struct response response_data;
    const std::vector<float> coordinates{4, 4};

    EXPECT_THAT([&]() {
        cppapi::fence(
            *double_datahandle,
            coordinate_system::INDEX,
            coordinates.data(),
            int(coordinates.size() / 2),
            NEAREST,
            nullptr,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate (4.000000,4.000000) is out of boundaries in dimension 0.")));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(DatahandleFenceTest, Fence_ANNOTATION_Single) {

    struct response response_data;
    const std::vector<float> coordinates{3, 2, 6, 4, 9, 6, 12, 8, 15, 10, 18, 12, 21, 14, 24, 16};
    const std::vector<float> check_coordinates{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7};

    cppapi::fence(
        *single_datahandle,
        coordinate_system::ANNOTATION,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {0, 0, 0};
    int high[3] = {8, 8, 32};
    check_fence(response_data, check_coordinates, low, high, 1, true);
}

TEST_F(DatahandleFenceTest, Fence_ANNOTATION_Double) {

    struct response response_data;
    const std::vector<float> coordinates{15, 10, 18, 12, 21, 14, 24, 16};
    const std::vector<float> check_coordinates{4, 4, 5, 5, 6, 6, 7, 7};

    cppapi::fence(
        *double_datahandle,
        coordinate_system::ANNOTATION,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {4, 4, 4};
    int high[3] = {8, 8, 32};
    check_fence(response_data, check_coordinates, low, high, 2, true);
}

TEST_F(DatahandleFenceTest, Fence_ANNOTATION_Fill_Single) {

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 27, 18};

    cppapi::fence(
        *single_datahandle,
        coordinate_system::ANNOTATION,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        &fill,
        &response_data
    );

    int low[3] = {0, 0, 0};
    int high[3] = {2, 2, 32};
    check_fence(response_data, coordinates, low, high, 1, false);
}

TEST_F(DatahandleFenceTest, Fence_ANNOTATION_Fill_Double) {

    struct response response_data;
    const std::vector<float> coordinates{-1, -1, 4, 4};

    cppapi::fence(
        *double_datahandle,
        coordinate_system::ANNOTATION,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        &fill,
        &response_data
    );

    int low[3] = {0, 0, 4};
    int high[3] = {2, 2, 32};
    check_fence(response_data, coordinates, low, high, 1, false);
}

TEST_F(DatahandleFenceTest, Fence_ANNOTATION_out_Of_Bounds_Single) {

    struct response response_data;
    const std::vector<float> coordinates{0, 0};

    EXPECT_THAT([&]() {
        cppapi::fence(
            *single_datahandle,
            coordinate_system::ANNOTATION,
            coordinates.data(),
            int(coordinates.size() / 2),
            NEAREST,
            nullptr,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate (0.000000,0.000000) is out of boundaries in dimension 0.")));
}

TEST_F(DatahandleFenceTest, Fence_ANNOTATION_out_Of_Bounds_Double) {

    struct response response_data;
    const std::vector<float> coordinates{4, 4};

    EXPECT_THAT([&]() {
        cppapi::fence(
            *double_datahandle,
            coordinate_system::ANNOTATION,
            coordinates.data(),
            int(coordinates.size() / 2),
            NEAREST,
            nullptr,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate (4.000000,4.000000) is out of boundaries in dimension 0.")));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(DatahandleFenceTest, Fence_CDP_Single) {

    struct response response_data;
    const std::vector<float> coordinates{2, 0, 7, 12, 12, 24, 17, 36, 22, 48, 27, 60, 32, 72, 37, 84};
    const std::vector<float> check_coordinates{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7};

    cppapi::fence(
        *single_datahandle,
        coordinate_system::CDP,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {0, 0, 0};
    int high[3] = {8, 8, 32};
    check_fence(response_data, check_coordinates, low, high, 1, true);
}

TEST_F(DatahandleFenceTest, Fence_CDP_Double) {

    struct response response_data;
    const std::vector<float> coordinates{22, 48, 27, 60, 32, 72, 37, 84};
    const std::vector<float> check_coordinates{4, 4, 5, 5, 6, 6, 7, 7};

    cppapi::fence(
        *double_datahandle,
        coordinate_system::CDP,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {4, 4, 4};
    int high[3] = {8, 8, 32};
    check_fence(response_data, check_coordinates, low, high, 2, true);
}

TEST_F(DatahandleFenceTest, Fence_CDP_Fill_Single) {

    struct response response_data;
    const std::vector<float> coordinates{-3, -12, 42, 96};

    cppapi::fence(
        *single_datahandle,
        coordinate_system::CDP,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        &fill,
        &response_data
    );

    int low[3] = {0, 0, 0};
    int high[3] = {2, 2, 32};
    check_fence(response_data, coordinates, low, high, 1, false);
}

TEST_F(DatahandleFenceTest, Fence_CDP_Fill_Double) {

    struct response response_data;
    const std::vector<float> coordinates{17, 36, 42, 96};

    cppapi::fence(
        *double_datahandle,
        coordinate_system::CDP,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        &fill,
        &response_data
    );

    int low[3] = {0, 0, 4};
    int high[3] = {2, 2, 32};
    check_fence(response_data, coordinates, low, high, 1, false);
}

TEST_F(DatahandleFenceTest, Fence_CDP_out_Of_Bounds_Single) {

    struct response response_data;
    const std::vector<float> coordinates{-3, -12};

    EXPECT_THAT([&]() {
        cppapi::fence(
            *single_datahandle,
            coordinate_system::CDP,
            coordinates.data(),
            int(coordinates.size() / 2),
            NEAREST,
            nullptr,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate (-3.000000,-12.000000) is out of boundaries in dimension 0.")));
}

TEST_F(DatahandleFenceTest, Fence_CDP_out_Of_Bounds_Double) {

    struct response response_data;
    const std::vector<float> coordinates{17, 36};

    EXPECT_THAT([&]() {
        cppapi::fence(
            *double_datahandle,
            coordinate_system::CDP,
            coordinates.data(),
            int(coordinates.size() / 2),
            NEAREST,
            nullptr,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate (17.000000,36.000000) is out of boundaries in dimension 0.")));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous tests

TEST_F(DatahandleFenceTest, Fence_INDEX_Reverse_Double) {

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const std::vector<float> check_coordinates{4, 4, 5, 5, 6, 6, 7, 7};

    cppapi::fence(
        *double_reverse_datahandle,
        coordinate_system::INDEX,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {4, 4, 4};
    int high[3] = {8, 8, 32};
    check_fence(response_data, check_coordinates, low, high, 2, true);
}

TEST_F(DatahandleFenceTest, Fence_INDEX_Empty_Double) {

    struct response response_data;
    const std::vector<float> coordinates{0, 0};

    EXPECT_THAT([&]() {
        cppapi::fence(
            *double_empty,
            coordinate_system::INDEX,
            coordinates.data(),
            int(coordinates.size() / 2),
            NEAREST,
            nullptr,
            &response_data
        );
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate (0.000000,0.000000) is out of boundaries in dimension 0.")));
}

TEST_F(DatahandleFenceTest, Fence_INDEX_Different_Size_Double) {

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const std::vector<float> check_coordinates{8, 8, 9, 9, 10, 10, 11, 11};

    cppapi::fence(
        *double_different_size,
        coordinate_system::INDEX,
        coordinates.data(),
        int(coordinates.size() / 2),
        NEAREST,
        nullptr,
        &response_data
    );

    int low[3] = {4, 4, 8};
    int high[3] = {8, 8, 36};
    check_fence(response_data, check_coordinates, low, high, 2, true);
}

} // namespace
