#include "cppapi.hpp"
#include "ctypes.h"
#include <iostream>
#include <sstream>

#include "attribute.hpp"
#include "metadatahandle.hpp"
#include "nlohmann/json.hpp"
#include "subvolume.hpp"
#include "utils.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const std::string REGULAR_DATA = "file://regular_8x3_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x3_cube.vds";
const std::string SHIFT_M4_DATA = "file://shift_minus4_8x3_cube.vds";

const std::string CREDENTIALS = "";

double norm(const std::vector<double>& vec) {
    double sum = 0.0;
    for (double component : vec) {
        sum += std::pow(component, 2);
    }
    return std::sqrt(sum);
}

class DoubleVolumeDataLayoutTest : public ::testing::Test {

    void SetUp() override {
        expected_intersect_metadata["crs"] = "\"utmXX\"; \"utmXX\"";
        expected_intersect_metadata["inputFileName"] = "regular_8x3_cube.segy; shift_4_8x3_cube.segy";
        expected_intersect_metadata["boundingBox"]["cdp"] = {{22.0, 48.0}, {49.0, 66.0}, {37.0, 84.0}, {10.0, 66.0}};
        expected_intersect_metadata["boundingBox"]["ij"] = {{0.0, 0.0}, {3.0, 0.0}, {3.0, 3.0}, {0.0, 3.0}};
        expected_intersect_metadata["boundingBox"]["ilxl"] = {{15, 10}, {24, 10}, {24, 16}, {15, 16}};
        expected_intersect_metadata["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 15.0f}, {"samples", 4}, {"stepsize", 3.0f}, {"unit", "unitless"}};
        expected_intersect_metadata["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 10.0f}, {"samples", 4}, {"stepsize", 2.0f}, {"unit", "unitless"}};
        expected_intersect_metadata["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 20.0f}, {"samples", 28}, {"stepsize", 4.0f}, {"unit", "ms"}};

        single_datasource = make_single_datasource(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str()
        );

        double_datasource = make_double_datasource(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            &inplace_addition
        );

        double_reverse_datasource = make_double_datasource(
            SHIFT_4_DATA.c_str(),
            CREDENTIALS.c_str(),
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            &inplace_addition
        );
    }

    void TearDown() override {
        delete single_datasource;
        delete double_datasource;
        delete double_reverse_datasource;
    }

public:
    nlohmann::json expected_intersect_metadata;

    SingleDataSource* single_datasource;
    DoubleDataSource* double_datasource;
    DoubleDataSource* double_reverse_datasource;
};

TEST_F(DoubleVolumeDataLayoutTest, Single_Metadata) {

    nlohmann::json expected;
    expected["crs"] = "\"utmXX\"";
    expected["inputFileName"] = "regular_8x3_cube.segy";
    expected["importTimeStamp"] = "2024-03-01T11:58:35.521Z";
    expected["boundingBox"]["cdp"] = {{2.0, 0.0}, {65.0, 42.0}, {37.0, 84.0}, {-26.0, 42.0}};
    expected["boundingBox"]["ij"] = {{0.0, 0.0}, {7.0, 0.0}, {7.0, 7.0}, {0.0, 7.0}};
    expected["boundingBox"]["ilxl"] = {{3, 2}, {24, 2}, {24, 16}, {3, 16}};
    expected["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 3.0f}, {"samples", 8}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    expected["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 2.0f}, {"samples", 8}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 4.0f}, {"samples", 32}, {"stepsize", 4.0f}, {"unit", "ms"}};

    struct response response_data;
    cppapi::metadata(*single_datasource, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data);

    EXPECT_EQ(metadata["crs"], expected["crs"]);
    EXPECT_EQ(metadata["inputFileName"], expected["inputFileName"]);
    EXPECT_EQ(metadata["boundingBox"], expected["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected["axis"]);
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Metadata) {

    struct response response_data;
    cppapi::metadata(*double_datasource, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data);

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], expected_intersect_metadata["inputFileName"]);
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);
}

TEST_F(DoubleVolumeDataLayoutTest, Reverse_Addition_Metadata) {

    struct response response_data;
    cppapi::metadata(*double_reverse_datasource, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data);

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], "shift_4_8x3_cube.segy; regular_8x3_cube.segy");
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);

}

} // namespace
