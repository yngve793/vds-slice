#include "cppapi.hpp"
#include "metadatahandle.hpp"
#include "nlohmann/json.hpp"
#include "gtest/gtest.h"

namespace {

/// Each value in the test files has the positional information encoded in little-endian.
/// Byte 0: Sample position
/// Byte 1: CrossLine position
/// Byte 2: InLine position
const std::string REGULAR_DATA = "file://regular_8x3_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x3_cube.vds";
const std::string SHIFT_8_BIG_DATA = "file://big_shift_8_32x3_cube.vds";

const std::string CREDENTIALS = "";

class DatahandleMetadataTest : public ::testing::Test {

    void SetUp() override {
        expected_intersect_metadata["crs"] = "utmXX";
        expected_intersect_metadata["boundingBox"]["cdp"] = {{22.0, 48.0}, {49.0, 66.0}, {37.0, 84.0}, {10.0, 66.0}};
        expected_intersect_metadata["boundingBox"]["ij"] = {{0.0, 0.0}, {3.0, 0.0}, {3.0, 3.0}, {0.0, 3.0}};
        expected_intersect_metadata["boundingBox"]["ilxl"] = {{15, 10}, {24, 10}, {24, 16}, {15, 16}};
        expected_intersect_metadata["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 15.0f}, {"samples", 4}, {"stepsize", 3.0f}, {"unit", "unitless"}};
        expected_intersect_metadata["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 10.0f}, {"samples", 4}, {"stepsize", 2.0f}, {"unit", "unitless"}};
        expected_intersect_metadata["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 20.0f}, {"samples", 28}, {"stepsize", 4.0f}, {"unit", "ms"}};

        single_datahandle = make_single_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str()
        );

        double_datahandle = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::ADDITION
        );

        double_subtraction_datahandle = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::SUBTRACTION
        );

        double_reverse_datahandle = make_double_datahandle(
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::ADDITION
        );

        double_empty = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_8_BIG_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::ADDITION
        );

        double_different_size = make_double_datahandle(
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_8_BIG_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::ADDITION
        );
    }

    void TearDown() override {
        delete single_datahandle;
        delete double_datahandle;
        delete double_subtraction_datahandle;
        delete double_reverse_datahandle;
        delete double_empty;
        delete double_different_size;
    }

public:
    nlohmann::json expected_intersect_metadata;

    SingleDataHandle* single_datahandle;
    DoubleDataHandle* double_datahandle;
    DoubleDataHandle* double_subtraction_datahandle;
    DoubleDataHandle* double_reverse_datahandle;
    DoubleDataHandle* double_empty;
    DoubleDataHandle* double_different_size;
};

TEST_F(DatahandleMetadataTest, Metadata_Single) {

    nlohmann::json expected;
    expected["crs"] = "utmXX";
    expected["inputFileName"] = "regular_8x3_cube.segy";
    expected["boundingBox"]["cdp"] = {{2.0, 0.0}, {65.0, 42.0}, {37.0, 84.0}, {-26.0, 42.0}};
    expected["boundingBox"]["ij"] = {{0.0, 0.0}, {7.0, 0.0}, {7.0, 7.0}, {0.0, 7.0}};
    expected["boundingBox"]["ilxl"] = {{3, 2}, {24, 2}, {24, 16}, {3, 16}};
    expected["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 3.0f}, {"samples", 8}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    expected["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 2.0f}, {"samples", 8}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 4.0f}, {"samples", 32}, {"stepsize", 4.0f}, {"unit", "ms"}};

    struct response response_data;
    cppapi::metadata(*single_datahandle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    EXPECT_EQ(metadata["crs"], expected["crs"]);
    EXPECT_EQ(metadata["inputFileName"], expected["inputFileName"]);
    EXPECT_EQ(metadata["boundingBox"], expected["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Double) {

    struct response response_data;
    cppapi::metadata(*double_datahandle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], "regular_8x3_cube.segy + shift_4_8x3_cube.segy");
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Subtraction_Double) {

    struct response response_data;
    cppapi::metadata(*double_subtraction_datahandle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], "regular_8x3_cube.segy - shift_4_8x3_cube.segy");
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Reverse_Double) {

    struct response response_data;
    cppapi::metadata(*double_reverse_datahandle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], "shift_4_8x3_cube.segy + regular_8x3_cube.segy");
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Empty_Double) {

    struct response response_data;
    cppapi::metadata(*double_empty, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    nlohmann::json empty_metadata;
    empty_metadata["inputFileName"] = "regular_8x3_cube.segy + big_shift_8_32x3_cube.segy";
    empty_metadata["boundingBox"]["cdp"] = {{42.0, 96.0}, {33.0, 90.0}, {37.0, 84.0}, {46.0, 90.0}};
    empty_metadata["boundingBox"]["ij"] = {{0.0, 0.0}, {-1.0, 0.0}, {-1.0, -1.0}, {0.0, -1.0}};
    empty_metadata["boundingBox"]["ilxl"] = {{27, 18}, {24, 18}, {24, 16}, {27, 16}};
    empty_metadata["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 27.0f}, {"samples", 0}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    empty_metadata["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 18.0f}, {"samples", 0}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    empty_metadata["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 36.0f}, {"samples", 24}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], empty_metadata["inputFileName"]);
    EXPECT_EQ(metadata["boundingBox"], empty_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], empty_metadata["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Different_Size_Double) {

    struct response response_data;
    cppapi::metadata(*double_different_size, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    nlohmann::json different_size_metadata;
    different_size_metadata["inputFileName"] = "shift_4_8x3_cube.segy + big_shift_8_32x3_cube.segy";
    different_size_metadata["boundingBox"]["cdp"] = {{42.0, 96.0}, {69.0, 114.0}, {57.0, 132.0}, {30.0, 114.0}};
    different_size_metadata["boundingBox"]["ij"] = {{0.0, 0.0}, {3.0, 0.0}, {3.0, 3.0}, {0.0, 3.0}};
    different_size_metadata["boundingBox"]["ilxl"] = {{27, 18}, {36, 18}, {36, 24}, {27, 24}};
    different_size_metadata["axis"][0] = {{"annotation", "Inline"}, {"max", 36.0f}, {"min", 27.0f}, {"samples", 4}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    different_size_metadata["axis"][1] = {{"annotation", "Crossline"}, {"max", 24.0f}, {"min", 18.0f}, {"samples", 4}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    different_size_metadata["axis"][2] = {{"annotation", "Sample"}, {"max", 144.0f}, {"min", 36.0f}, {"samples", 28}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], different_size_metadata["inputFileName"]);
    EXPECT_EQ(metadata["boundingBox"], different_size_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], different_size_metadata["axis"]);
}

} // namespace
