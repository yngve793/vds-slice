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
    expected["importTimeStamp"] = "2024-03-01T11:58:35.521Z";
    expected["boundingBox"]["cdp"] = {{2.0, 0.0}, {65.0, 42.0}, {37.0, 84.0}, {-26.0, 42.0}};
    expected["boundingBox"]["ij"] = {{0.0, 0.0}, {7.0, 0.0}, {7.0, 7.0}, {0.0, 7.0}};
    expected["boundingBox"]["ilxl"] = {{3, 2}, {24, 2}, {24, 16}, {3, 16}};
    expected["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 3.0f}, {"samples", 8}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    expected["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 2.0f}, {"samples", 8}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 4.0f}, {"samples", 32}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(meta["crs"], expected["crs"]);
    EXPECT_EQ(meta["inputFileName"], expected["inputFileName"]);
    // EXPECT_EQ(meta["importTimeStamp"], expected["importTimeStamp"]);
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
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 20.0f}, {"samples", 28}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(meta["crs"], expected["crs"]);
    EXPECT_EQ(meta["inputFileName"], expected["inputFileName"]);
    // EXPECT_EQ(meta["importTimeStamp"], expected["importTimeStamp"]);
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
    expected["importTimeStamp"] = "2024-03-01T11:58:35.521Z; 2024-03-01T11:58:36.635Z";
    expected["boundingBox"]["cdp"] = {{2.0, 0.0}, {29.0, 18.0}, {17.0, 36.0}, {-10.0, 18.0}};
    expected["boundingBox"]["ij"] = {{0.0, 0.0}, {3.0, 0.0}, {3.0, 3.0}, {0.0, 3.0}};
    expected["boundingBox"]["ilxl"] = {{15, 10}, {24, 10}, {24, 16}, {15, 16}};
    expected["axis"][0] = {{"annotation", "Inline"}, {"max", 24.0f}, {"min", 15.0f}, {"samples", 4}, {"stepsize", 3.0f}, {"unit", "unitless"}};
    expected["axis"][1] = {{"annotation", "Crossline"}, {"max", 16.0f}, {"min", 10.0f}, {"samples", 4}, {"stepsize", 2.0f}, {"unit", "unitless"}};
    expected["axis"][2] = {{"annotation", "Sample"}, {"max", 128.0f}, {"min", 20.0f}, {"samples", 28}, {"stepsize", 4.0f}, {"unit", "ms"}};

    EXPECT_EQ(meta["crs"], expected["crs"]);
    EXPECT_EQ(meta["inputFileName"], expected["inputFileName"]);
    // EXPECT_EQ(meta["importTimeStamp"], expected["importTimeStamp"]);
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

    std::vector<int> iline_array = {15, 18, 21, 24};
    std::vector<int> xline_array = {10, 12, 14, 16};
    std::vector<int> sample_array(28);

    for (int i = 0; i < sample_array.size(); i++) {
        sample_array[i] = 20 + i * 4;
    }

    int counter = 0;
    for (int ic = 0; ic < iline_array.size(); ++ic) {
        for (int s = 0; s < sample_array.size(); ++s) {
            int value = int((*(float*)&response_data.data[counter * sizeof(float)] / 2) + 0.5f);
            int sample = value & 0xFF;
            int xline = (value & 0xFF00) >> 8;
            int iline = (value & 0xFF0000) >> 16;

            // std::cout << counter << " " << iline << " " << xline << " " << " " << sample << std::endl;

            EXPECT_EQ(iline, iline_array[ic]);
            EXPECT_EQ(xline, xline_array[ic]);
            EXPECT_EQ(sample, sample_array[s]);
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

    std::vector<int> iline_array = {15, 18, 21, 24};
    std::vector<int> xline_array = {10, 12, 14, 16};
    std::vector<int> sample_array(28);

    for (int i = 0; i < sample_array.size(); i++) {
        sample_array[i] = 20 + i * 4;
    }

    int counter = 0;
    for (int ic = 0; ic < iline_array.size(); ++ic) {
        for (int s = 0; s < sample_array.size(); ++s) {
            int value = int((*(float*)&response_data.data[counter * sizeof(float)] / 2) + 0.5f);
            int sample = value & 0xFF;
            int xline = (value & 0xFF00) >> 8;
            int iline = (value & 0xFF0000) >> 16;

            // std::cout << counter << " " << iline << " " << xline << " " << " " << sample << std::endl;

            EXPECT_EQ(iline, iline_array[ic]);
            EXPECT_EQ(xline, xline_array[ic]);
            EXPECT_EQ(sample, sample_array[s]);
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

TEST_F(DoubleVolumeDataLayoutTest, Single_Attribute) {

    SingleDataSource* datasource = make_single_datasource(
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str()
    );

    std::vector<double> iline_vector = {-2.0, 3.0};
    std::vector<double> xline_vector = {3.0, 2.0};

    static Grid default_grid = Grid(2, 0, norm(iline_vector) * 3, norm(xline_vector) * 2, 33.69);

    static constexpr float fill = -999.25;
    static constexpr std::size_t nrows = 8;
    static constexpr std::size_t ncols = 8;
    static constexpr std::size_t size = nrows * ncols;

    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> primary_surface_data(nrows * ncols, 36.0f);
    static std::vector<float> bottom_surface_data(nrows * ncols, 52.0f);

    RegularSurface primary_surface =
        RegularSurface(primary_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface top_surface =
        RegularSurface(top_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface bottom_surface =
        RegularSurface(bottom_surface_data.data(), nrows, ncols, default_grid, fill);

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);

    std::vector<int> iline_array(8);
    std::vector<int> xline_array(8);
    std::vector<int> sample_array(11);

    for (int i = 0; i < iline_array.size(); i++) {
        iline_array[i] = 3 + i * 3;
        xline_array[i] = 2 + i * 2;
    }

    for (int i = 0; i < sample_array.size(); i++) {
        sample_array[i] = 20 + i * 4;
    }

    int counter = 0;
    for (int ic = 0; ic < iline_array.size(); ++ic) {
        for (int xc = 0; xc < xline_array.size(); ++xc) {
            RawSegment rs = subvolume->vertical_segment(counter);

            int s = 0;
            for (auto it = rs.begin(); it != rs.end(); ++it) {
                int value = int(*it + 0.5f);
                int sample = value & 0xFF;
                int xline = (value & 0xFF00) >> 8;
                int iline = (value & 0xFF0000) >> 16;

                EXPECT_EQ(iline, iline_array[ic]);
                EXPECT_EQ(xline, xline_array[xc]);
                EXPECT_EQ(sample, sample_array[s]);
                s += 1;
            }
            counter += 1;
        }
    }

    delete subvolume;
    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Offset_Attribute) {

    DoubleDataSource* datasource = make_double_datasource(
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    std::vector<double> iline_vector = {-2.0, 3.0};
    std::vector<double> xline_vector = {3.0, 2.0};

    static Grid default_grid = Grid(2, 0, norm(iline_vector) * 3, norm(xline_vector) * 2, 33.69);

    static constexpr float fill = -999.25;
    static constexpr std::size_t nrows = 4;
    static constexpr std::size_t ncols = 4;
    static constexpr std::size_t size = nrows * ncols;

    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> primary_surface_data(nrows * ncols, 36.0f);
    static std::vector<float> bottom_surface_data(nrows * ncols, 52.0f);

    RegularSurface primary_surface =
        RegularSurface(primary_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface top_surface =
        RegularSurface(top_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface bottom_surface =
        RegularSurface(bottom_surface_data.data(), nrows, ncols, default_grid, fill);

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);

    std::vector<int> iline_array(nrows);
    std::vector<int> xline_array(ncols);
    std::vector<int> sample_array(28);

    for (int i = 0; i < iline_array.size(); i++) {
        iline_array[i] = 15 + i * 3;
        xline_array[i] = 10 + i * 2;
    }

    for (int i = 0; i < sample_array.size(); i++) {
        sample_array[i] = 20 + i * 4;
    }

    int counter = 0;
    for (int ic = 0; ic < iline_array.size(); ++ic) {
        for (int xc = 0; xc < xline_array.size(); ++xc) {
            RawSegment rs = subvolume->vertical_segment(counter);

            int s = 0;
            for (auto it = rs.begin(); it != rs.end(); ++it) {
                int value = int(((*it) / 2) + 0.5f);
                int sample = value & 0xFF;
                int xline = (value & 0xFF00) >> 8;
                int iline = (value & 0xFF0000) >> 16;

                EXPECT_EQ(iline, iline_array[ic]);
                EXPECT_EQ(xline, xline_array[xc]);
                EXPECT_EQ(sample, sample_array[s]);
                s += 1;
            }
            counter += 1;
        }
    }

    delete subvolume;
    delete datasource;
}

TEST_F(DoubleVolumeDataLayoutTest, Addition_Offset_reverse_Attribute) {

    DoubleDataSource* datasource = make_double_datasource(
        SHIFT_4_DATA.c_str(),
        CREDENTIALS.c_str(),
        REGULAR_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    std::vector<double> iline_vector = {-2.0, 3.0};
    std::vector<double> xline_vector = {3.0, 2.0};

    static Grid default_grid = Grid(2, 0, norm(iline_vector) * 3, norm(xline_vector) * 2, 33.69);

    static constexpr float fill = -999.25;
    static constexpr std::size_t nrows = 4;
    static constexpr std::size_t ncols = 4;
    static constexpr std::size_t size = nrows * ncols;

    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> primary_surface_data(nrows * ncols, 36.0f);
    static std::vector<float> bottom_surface_data(nrows * ncols, 52.0f);

    RegularSurface primary_surface =
        RegularSurface(primary_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface top_surface =
        RegularSurface(top_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface bottom_surface =
        RegularSurface(bottom_surface_data.data(), nrows, ncols, default_grid, fill);

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);

    std::vector<int> iline_array(nrows);
    std::vector<int> xline_array(ncols);
    std::vector<int> sample_array(28);

    for (int i = 0; i < iline_array.size(); i++) {
        iline_array[i] = 15 + i * 3;
        xline_array[i] = 10 + i * 2;
    }

    for (int i = 0; i < sample_array.size(); i++) {
        sample_array[i] = 20 + i * 4;
    }

    int counter = 0;
    for (int ic = 0; ic < iline_array.size(); ++ic) {
        for (int xc = 0; xc < xline_array.size(); ++xc) {
            RawSegment rs = subvolume->vertical_segment(counter);

            int s = 0;
            for (auto it = rs.begin(); it != rs.end(); ++it) {
                int value = int(((*it) / 2) + 0.5f);
                int sample = value & 0xFF;
                int xline = (value & 0xFF00) >> 8;
                int iline = (value & 0xFF0000) >> 16;

                EXPECT_EQ(iline, iline_array[ic]);
                EXPECT_EQ(xline, xline_array[xc]);
                EXPECT_EQ(sample, sample_array[s]);
                s += 1;
            }
            counter += 1;
        }
    }

    delete subvolume;
    delete datasource;
}

} // namespace
