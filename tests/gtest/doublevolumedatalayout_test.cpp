#include "cppapi.hpp"
#include "ctypes.h"
#include <iostream>
#include <sstream>

#include "nlohmann/json.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const std::string REGULAR_DATA = "file://regular_8x3_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x3_cube.vds";
const std::string SHIFT_M4_DATA = "file://shift_minus4_8x3_cube.vds";

const std::string CREDENTIALS = "";

class DoubleVolumeDataLayoutTest : public ::testing::Test {
};

TEST_F(DoubleVolumeDataLayoutTest, Single_Metadata) {

    SingleDataSource* datasource = make_single_datasource(
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str()
    );

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const coordinate_system c_system = coordinate_system::INDEX;
    const int coordinate_size = int(coordinates.size() / 2);
    const enum interpolation_method interpolation = NEAREST;
    const float fill = -999.25;

    cppapi::metadata(*datasource, &response_data);


    nlohmann::json meta = nlohmann::json::parse(response_data.data);

    nlohmann::json expected;
    expected["crs"] = "\"utmXX\"";
    expected["inputFileName"] = "regular_8x3_cube.segy";
    expected["importTimeStamp"] = "2024-02-28T11:26:19.493Z";
    expected["boundingBox"]["cdp"] = {{2.0, 0.0}, {65.0, 42.0}, {37.0, 84.0}, {-26.0, 42.0}};
    expected["boundingBox"]["ij"] = {{0.0, 0.0}, {7.0, 0.0}, {7.0, 7.0}, {0.0, 7.0}};
    expected["boundingBox"]["ilxl"] = {{3, 2}, {24, 2}, {24, 16}, {3, 16}};
    expected["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 3.0f}, {"samples", 8}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    expected["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 2.0f}, {"samples", 8}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 32.0f}, {"min", 4.0f}, {"samples", 8}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(meta["crs"], expected["crs"]);
    EXPECT_EQ(meta["inputFileName"], expected["inputFileName"]);
    EXPECT_EQ(meta["importTimeStamp"], expected["importTimeStamp"]);
    EXPECT_EQ(meta["boundingBox"], expected["boundingBox"]);
    EXPECT_EQ(meta["axis"], expected["axis"]);

    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Metadata) {

    DoubleDataSource* datasource = make_double_datasource(
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const coordinate_system c_system = coordinate_system::INDEX;
    const int coordinate_size = int(coordinates.size() / 2);
    const enum interpolation_method interpolation = NEAREST;
    const float fill = -999.25;

    cppapi::metadata(*datasource, &response_data);

    nlohmann::json meta = nlohmann::json::parse(response_data.data);

    nlohmann::json expected;
    expected["crs"] = "\"utmXX\"; \"utmXX\"";
    expected["inputFileName"] = "shift_4_8x3_cube.segy; regular_8x3_cube.segy";
    expected["importTimeStamp"] = "2024-02-28T11:26:20.634Z; 2024-02-28T11:26:19.493Z";
    expected["boundingBox"]["cdp"] = {{2.0, 0.0}, {29.0, 18.0}, {17.0, 36.0}, {-10.0, 18.0}};
    expected["boundingBox"]["ij"] = {{0.0, 0.0}, {3.0, 0.0}, {3.0, 3.0}, {0.0, 3.0}};
    expected["boundingBox"]["ilxl"] = {{15, 10}, {24, 10}, {24, 16}, {15, 16}};
    expected["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 15.0f}, {"samples", 4}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    expected["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 10.0f}, {"samples", 4}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 32.0f}, {"min", 20.0f}, {"samples", 4}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(meta["crs"], expected["crs"]);
    EXPECT_EQ(meta["inputFileName"], expected["inputFileName"]);
    EXPECT_EQ(meta["importTimeStamp"], expected["importTimeStamp"]);
    EXPECT_EQ(meta["boundingBox"], expected["boundingBox"]);
    EXPECT_EQ(meta["axis"], expected["axis"]);

    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_reverse_Metadata) {

    DoubleDataSource* datasource = make_double_datasource(
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const coordinate_system c_system = coordinate_system::INDEX;
    const int coordinate_size = int(coordinates.size() / 2);
    const enum interpolation_method interpolation = NEAREST;
    const float fill = -999.25;

    cppapi::metadata(*datasource, &response_data);

    nlohmann::json meta = nlohmann::json::parse(response_data.data);

    nlohmann::json expected;
    expected["crs"] = "\"utmXX\"; \"utmXX\"";
    expected["inputFileName"] = "regular_8x3_cube.segy; shift_4_8x3_cube.segy";
    expected["importTimeStamp"] = "2024-02-28T11:26:19.493Z; 2024-02-28T11:26:20.634Z";
    expected["boundingBox"]["cdp"] = {{2.0, 0.0}, {29.0, 18.0}, {17.0, 36.0}, {-10.0, 18.0}};
    expected["boundingBox"]["ij"] = {{0.0, 0.0}, {3.0, 0.0}, {3.0, 3.0}, {0.0, 3.0}};
    expected["boundingBox"]["ilxl"] = {{15, 10}, {24, 10}, {24, 16}, {15, 16}};
    expected["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 15.0f}, {"samples", 4}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    expected["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 10.0f}, {"samples", 4}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 32.0f}, {"min", 20.0f}, {"samples", 4}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(meta["crs"], expected["crs"]);
    EXPECT_EQ(meta["inputFileName"], expected["inputFileName"]);
    EXPECT_EQ(meta["importTimeStamp"], expected["importTimeStamp"]);
    EXPECT_EQ(meta["boundingBox"], expected["boundingBox"]);
    EXPECT_EQ(meta["axis"], expected["axis"]);

    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Offset_reverse_Fence) {

    DoubleDataSource* datasource = make_double_datasource(
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const coordinate_system c_system = coordinate_system::INDEX;
    const int coordinate_size = int(coordinates.size() / 2);
    const enum interpolation_method interpolation = NEAREST;
    const float fill = -999.25;

    cppapi::fence(
        *datasource,
        c_system,
        coordinates.data(),
        coordinate_size,
        interpolation,
        &fill,
        &response_data
    );

    int iline_array[] = {15, 18, 21, 24};
    int xline_array[] = {10, 12, 14, 16};
    int sample_array[] = {20, 24, 28, 32};

    int counter = 0;
    for (int tc = 0; tc < sizeof(iline_array) / sizeof(iline_array[0]); ++tc) {
        for (int s = 0; s < sizeof(sample_array) / sizeof(sample_array[0]); ++s) {

            int value = int(*(float*)&response_data.data[counter * sizeof(float)]) + 0.5f;
            int sample = value & 0xFF;
            int xline = value & 0xFF00;
            int iline = value & 0xFF0000;
            iline = iline >> 16;
            xline = xline >> 8;

            EXPECT_EQ((int)(iline / 2), iline_array[tc]);
            EXPECT_EQ((int)(xline / 2), xline_array[tc]);
            EXPECT_EQ((int)(sample / 2), sample_array[s]);

            counter += 1;
        }
    }

    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Offset_Fence) {

    DoubleDataSource* datasource = make_double_datasource(
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    struct response response_data;
    const std::vector<float> coordinates{0, 0, 1, 1, 2, 2, 3, 3};
    const coordinate_system c_system = coordinate_system::INDEX;
    const int coordinate_size = int(coordinates.size() / 2);
    const enum interpolation_method interpolation = NEAREST;
    const float fill = -999.25;

    cppapi::fence(
        *datasource,
        c_system,
        coordinates.data(),
        coordinate_size,
        interpolation,
        &fill,
        &response_data
    );

    int iline_array[] = {15, 18, 21, 24};
    int xline_array[] = {10, 12, 14, 16};
    int sample_array[] = {20, 24, 28, 32};

    int counter = 0;
    for (int tc = 0; tc < sizeof(iline_array) / sizeof(iline_array[0]); ++tc) {
        for (int s = 0; s < sizeof(sample_array) / sizeof(sample_array[0]); ++s) {

            int value = int(*(float*)&response_data.data[counter * sizeof(float)]) + 0.5f;
            int sample = value & 0xFF;
            int xline = value & 0xFF00;
            int iline = value & 0xFF0000;
            iline = iline >> 16;
            xline = xline >> 8;

            EXPECT_EQ((int)(iline / 2), iline_array[tc]);
            EXPECT_EQ((int)(xline / 2), xline_array[tc]);
            EXPECT_EQ((int)(sample / 2), sample_array[s]);

            counter += 1;
        }
    }

    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Offset_Slice) {

    DoubleDataSource* datasource = make_double_datasource(
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    Direction const direction(axis_name::TIME);
    int lineno = 24;

    std::vector<Bound> slice_bounds;

    struct response response_data;

    cppapi::slice(
        *datasource,
        direction,
        lineno,
        slice_bounds,
        &response_data
    );

    std::size_t nr_of_values = (std::size_t)(response_data.size / sizeof(float));

    int iline_array[] = {15, 18, 21, 24};
    int xline_array[] = {10, 12, 14, 16};

    int counter = 0;
    for (int il = 0; il < sizeof(iline_array) / sizeof(iline_array[0]); ++il) {
        for (int xl = 0; xl < sizeof(xline_array) / sizeof(xline_array[0]); ++xl) {

            int value = int(*(float*)&response_data.data[counter * sizeof(float)]) + 0.5f;
            int sample = value & 0xFF;
            int xline = value & 0xFF00;
            int iline = value & 0xFF0000;
            iline = iline >> 16;
            xline = xline >> 8;

            EXPECT_EQ((int)(iline / 2), iline_array[il]);
            EXPECT_EQ((int)(xline / 2), xline_array[xl]);
            EXPECT_EQ((int)(sample / 2), lineno);

            counter += 1;
        }
    }

    // delete subvolume;
    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Offset_reverse_Slice) {

    DoubleDataSource* datasource = make_double_datasource(
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    Direction const direction(axis_name::TIME);
    int lineno = 24;

    std::vector<Bound> slice_bounds;

    struct response response_data;

    cppapi::slice(
        *datasource,
        direction,
        lineno,
        slice_bounds,
        &response_data
    );

    std::size_t nr_of_values = (std::size_t)(response_data.size / sizeof(float));

    int iline_array[] = {15, 18, 21, 24};
    int xline_array[] = {10, 12, 14, 16};

    int counter = 0;
    for (int il = 0; il < sizeof(iline_array) / sizeof(iline_array[0]); ++il) {
        for (int xl = 0; xl < sizeof(xline_array) / sizeof(xline_array[0]); ++xl) {

            int value = int(*(float*)&response_data.data[counter * sizeof(float)]) + 0.5f;
            int sample = value & 0xFF;
            int xline = value & 0xFF00;
            int iline = value & 0xFF0000;
            iline = iline >> 16;
            xline = xline >> 8;

            EXPECT_EQ((int)(iline / 2), iline_array[il]);
            EXPECT_EQ((int)(xline / 2), xline_array[xl]);
            EXPECT_EQ((int)(sample / 2), lineno);

            counter += 1;
        }
    }

    // delete subvolume;
    delete datasource;
}

} // namespace
