#include "cppapi.hpp"
#include "metadatahandle.hpp"
#include "nlohmann/json.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

/// Each value in the test files has the positional information encoded in little-endian.
/// Byte 0: Sample position
/// Byte 1: CrossLine position
/// Byte 2: InLine position
const std::string REGULAR_DATA = "file://regular_8x2_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x2_cube.vds";
const std::string SHIFT_8_BIG_DATA = "file://shift_8_32x3_cube.vds";

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
        delete double_different_size;
    }

public:
    nlohmann::json expected_intersect_metadata;

    SingleDataHandle* single_datahandle;
    DoubleDataHandle* double_datahandle;
    DoubleDataHandle* double_subtraction_datahandle;
    DoubleDataHandle* double_reverse_datahandle;
    DoubleDataHandle* double_different_size;
};

TEST_F(DatahandleMetadataTest, Metadata_Single) {

    nlohmann::json expected;
    expected["crs"] = "utmXX";
    expected["inputFileName"] = "regular_8x2_cube.segy";
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
    EXPECT_EQ(metadata["inputFileName"], "regular_8x2_cube.segy + shift_4_8x2_cube.segy");
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Subtraction_Double) {

    struct response response_data;
    cppapi::metadata(*double_subtraction_datahandle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], "regular_8x2_cube.segy - shift_4_8x2_cube.segy");
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Reverse_Double) {

    struct response response_data;
    cppapi::metadata(*double_reverse_datahandle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], "shift_4_8x2_cube.segy + regular_8x2_cube.segy");
    EXPECT_EQ(metadata["boundingBox"], expected_intersect_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], expected_intersect_metadata["axis"]);
}

TEST_F(DatahandleMetadataTest, Metadata_Different_Size_Double) {

    struct response response_data;
    cppapi::metadata(*double_different_size, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    nlohmann::json different_size_metadata;
    different_size_metadata["inputFileName"] = "shift_4_8x2_cube.segy + shift_8_32x3_cube.segy";
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

TEST_F(DatahandleMetadataTest, Metadata_One_crossLine_Single) {

    const std::string SINGLE_XLINE_DATA = "file://10_single_xline.vds";

    EXPECT_THAT([&]() {
        SingleDataHandle* single_xline_handle = make_single_datahandle(
            SINGLE_XLINE_DATA.c_str(),
            CREDENTIALS.c_str()
        );
        delete single_xline_handle;
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Unsupported VDS, expect at least two crossLines, got 1")));
}

TEST_F(DatahandleMetadataTest, Metadata_One_Sample_Single) {

    const std::string SINGLE_SAMPLE_DATA = "file://10_single_sample.vds";
    SingleDataHandle* single_sample_handle;

    EXPECT_THAT([&]() {
        SingleDataHandle* single_sample_handle = make_single_datahandle(
            SINGLE_SAMPLE_DATA.c_str(),
            CREDENTIALS.c_str()
        );
        delete single_sample_handle;
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Unsupported VDS, expect at least two samples, got 1")));
}

TEST_F(DatahandleMetadataTest, Metadata_Minimum_Cube_Single) {

    const std::string MINIMUM_CUBE = "file://10_min_dimensions.vds";

    SingleDataHandle* single_minimum_handle = make_single_datahandle(
        MINIMUM_CUBE.c_str(),
        CREDENTIALS.c_str()
    );

    struct response response_data;
    cppapi::metadata(*single_minimum_handle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    nlohmann::json different_size_metadata;
    different_size_metadata["inputFileName"] = "10_min_dimensions.segy";
    different_size_metadata["boundingBox"]["cdp"] = {{2.0, 0.0}, {8.0, 4.0}, {6.0, 7.0}, {0.0, 3.0}};
    different_size_metadata["boundingBox"]["ij"] = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
    different_size_metadata["boundingBox"]["ilxl"] = {{1, 10}, {3, 10}, {3, 11}, {1, 11}};
    different_size_metadata["axis"][0] = {{"annotation", "Inline"}, {"max", 3.0f}, {"min", 1.0f}, {"samples", 2}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    different_size_metadata["axis"][1] = {{"annotation", "Crossline"}, {"max", 11.0f}, {"min", 10.0f}, {"samples", 2}, {"stepsize", 1.0f}, {"unit", "unitless"}};
    different_size_metadata["axis"][2] = {{"annotation", "Sample"}, {"max", 8.0f}, {"min", 4.0f}, {"samples", 2}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], different_size_metadata["inputFileName"]);
    EXPECT_EQ(metadata["boundingBox"], different_size_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], different_size_metadata["axis"]);

    delete single_minimum_handle;
}

TEST_F(DatahandleMetadataTest, Metadata_One_inLine_Double) {

    EXPECT_THAT([&]() {
        const std::string SHIFT_7_INLINE_DATA = "file://shift_7_inLine_8x2_cube.vds";

        DoubleDataHandle* double_inline_datahandle = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_7_INLINE_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::SUBTRACTION
        );

        delete double_inline_datahandle;
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Unsupported VDS pair, expect that the intersection contains at least two inLines, got 1")));
}

TEST_F(DatahandleMetadataTest, Metadata_One_crossLine_Double) {

    EXPECT_THAT([&]() {
        const std::string SHIFT_7_CROSSLINE_DATA = "file://shift_7_xLine_8x2_cube.vds";

        DoubleDataHandle* double_xline_datahandle = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_7_CROSSLINE_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::SUBTRACTION
        );

        delete double_xline_datahandle;
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Unsupported VDS pair, expect that the intersection contains at least two crossLines, got 1")));
}

TEST_F(DatahandleMetadataTest, Metadata_One_Sample_Double) {

    EXPECT_THAT([&]() {
        const std::string SHIFT_31_SAMPLE_DATA = "file://shift_31_Sample_8x2_cube.vds";

        DoubleDataHandle* double_sample_datahandle = make_double_datahandle(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str(),
            SHIFT_31_SAMPLE_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::SUBTRACTION
        );

        delete double_sample_datahandle;
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Unsupported VDS pair, expect that the intersection contains at least two samples, got 1")));
}

TEST_F(DatahandleMetadataTest, Metadata_Minimum_Cube_Double) {

    const std::string SHIFT_6_CUBE = "file://shift_6_8x2_cube.vds";

    DoubleDataHandle* double_minimum_datahandle = make_double_datahandle(
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        SHIFT_6_CUBE.c_str(),
        CREDENTIALS.c_str(),
        binary_operator::SUBTRACTION
    );

    struct response response_data;
    cppapi::metadata(*double_minimum_datahandle, &response_data);
    nlohmann::json metadata = nlohmann::json::parse(response_data.data, response_data.data + response_data.size);

    nlohmann::json different_size_metadata;
    different_size_metadata["inputFileName"] = "regular_8x2_cube.segy - shift_6_8x2_cube.segy";
    different_size_metadata["boundingBox"]["cdp"] = {{32.0, 72.0}, {41.0, 78.0}, {37.0, 84.0}, {28.0, 78.0}};
    different_size_metadata["boundingBox"]["ij"] = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
    different_size_metadata["boundingBox"]["ilxl"] = {{21, 14}, {24, 14}, {24, 16}, {21, 16}};
    different_size_metadata["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 21.0f}, {"samples", 2}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    different_size_metadata["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 14.0f}, {"samples", 2}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    different_size_metadata["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 124.0f}, {"samples", 2}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(metadata["crs"], this->expected_intersect_metadata["crs"]);
    EXPECT_EQ(metadata["inputFileName"], different_size_metadata["inputFileName"]);
    EXPECT_EQ(metadata["boundingBox"], different_size_metadata["boundingBox"]);
    EXPECT_EQ(metadata["axis"], different_size_metadata["axis"]);

    delete double_minimum_datahandle;
}

TEST_F(DatahandleMetadataTest, Metadata_CRS_Double) {

    const std::string DEFAULT_DATA = "file://10_samples_default.vds";
    const std::string DEFAULT_CRS_DATA = "file://10_default_crs.vds";

    EXPECT_THAT([&]() {
        DoubleDataHandle* double_crs_datahandle = make_double_datahandle(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str(),
            DEFAULT_CRS_DATA.c_str(),
            CREDENTIALS.c_str(),
            binary_operator::SUBTRACTION
        );

        delete double_crs_datahandle;
    },
                testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr("Coordinate reference system (CRS) mismatch: utmXX versus utmXX_modified")));
}

} // namespace
