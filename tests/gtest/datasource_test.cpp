#include "cppapi.hpp"
#include "ctypes.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const std::string DEFAULT_DATA = "file://dimensions_data_il_10_xl_20_s_30.vds";
const std::string EXTRA_ILINE_DATA = "file://dimensions_data_il_11_xl_20_s_30.vds";
const std::string EXTRA_XLINE_DATA = "file://dimensions_data_il_10_xl_21_s_30.vds";
const std::string EXTRA_SAMPLE_DATA = "file://dimensions_data_il_10_xl_20_s_31.vds";
const std::string ALTERED_OFFSET_DATA = "file://dimensions_data_il_10_xl_20_s_30_offset.vds";
const std::string ALTERED_STEPSIZE_DATA = "file://dimensions_data_il_10_xl_20_s_30_stepsize.vds";
const std::string DOUBLE_VALUE_DATA = "file://dimensions_data_il_10_xl_20_s_30_double.vds";
const std::string CREDENTIALS = "";

const float DELTA = 0.00001;
const int EXPECTED_TRACE_LENGTH = 7;

Grid default_grid = Grid(22, 33, 2, 3, 0);

class DataSourceTest : public ::testing::Test {
protected:
    DoubleDataSource* datasource;
    SingleDataSource* datasource_A;
    SurfaceBoundedSubVolume* subvolume_A;
    static constexpr int nrows = 10;
    static constexpr int ncols = 20;
    static constexpr std::size_t size = nrows * ncols;

    std::array<float, size> top_surface_data;
    std::array<float, nrows * ncols> primary_surface_data;
    std::array<float, size> bottom_surface_data;

    static constexpr float fill = -999.25;

    RegularSurface primary_surface =
        RegularSurface(primary_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface top_surface =
        RegularSurface(top_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface bottom_surface =
        RegularSurface(bottom_surface_data.data(), nrows, ncols, default_grid, fill);

    void SetUp() override {
        datasource_reference = make_single_datasource(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str()
        );

        for (int i = 0; i < size; ++i) {
            top_surface_data[i] = 19.0;
            primary_surface_data[i] = 20.0;
            bottom_surface_data[i] = 29.0;
        };

        subvolume_reference = make_subvolume(
            datasource_reference->get_metadata(), primary_surface, top_surface, bottom_surface
        );

        cppapi::fetch_subvolume(*datasource_reference, *subvolume_reference, NEAREST, 0, size);
    }

    void TearDown() override {
        delete subvolume_reference;
        delete datasource_reference;
    }
};

TEST_F(DataSourceTest, ILineMismatch) {
    const std::string EXPECTED_MSG = "Axis: Inline: Mismatch in number of samples: 10 != 11";

    EXPECT_THAT([&]() {         
            DoubleDataSource *datasource = make_double_datasource(
                DEFAULT_DATA.c_str(),
                CREDENTIALS.c_str(),
                EXTRA_ILINE_DATA.c_str(),
                CREDENTIALS.c_str(),
                &inplace_subtraction);
        delete datasource; }, testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr(EXPECTED_MSG)));
}

TEST_F(DataSourceTest, XLineMismatch) {
    const std::string EXPECTED_MSG = "Axis: Crossline: Mismatch in number of samples: 20 != 21";

    EXPECT_THAT([&]() {         
            DoubleDataSource *datasource = make_double_datasource(
                DEFAULT_DATA.c_str(),
                CREDENTIALS.c_str(),
                EXTRA_XLINE_DATA.c_str(),
                CREDENTIALS.c_str(),
                &inplace_subtraction);
        delete datasource; }, testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr(EXPECTED_MSG)));
}

TEST_F(DataSourceTest, SamplesMismatch) {
    const std::string EXPECTED_MSG = "Axis: Sample: Mismatch in number of samples: 30 != 31";

    EXPECT_THAT([&]() {         
            DoubleDataSource *datasource = make_double_datasource(
                DEFAULT_DATA.c_str(),
                CREDENTIALS.c_str(),
                EXTRA_SAMPLE_DATA.c_str(),
                CREDENTIALS.c_str(),
                &inplace_subtraction);
        delete datasource; }, testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr(EXPECTED_MSG)));
}

TEST_F(DataSourceTest, OffsetMismatch) {
    const std::string EXPECTED_MSG = "Axis: Inline: Mismatch in min value: 22.000000 != 25.000000";

    EXPECT_THAT([&]() {         
            DoubleDataSource *datasource = make_double_datasource(
                DEFAULT_DATA.c_str(),
                CREDENTIALS.c_str(),
                ALTERED_OFFSET_DATA.c_str(),
                CREDENTIALS.c_str(),
                &inplace_subtraction);
        delete datasource; }, testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr(EXPECTED_MSG)));
}

TEST_F(DataSourceTest, StepSizeMismatch) {
    // Changing the stepsize causes the max value to change. Max value is checked before stepsize.
    const std::string EXPECTED_MSG = "Axis: Inline: Mismatch in max value: 40.000000 != 139.000000";

    EXPECT_THAT([&]() {         
            DoubleDataSource *datasource = make_double_datasource(
                DEFAULT_DATA.c_str(),
                CREDENTIALS.c_str(),
                ALTERED_STEPSIZE_DATA.c_str(),
                CREDENTIALS.c_str(),
                &inplace_subtraction);
        delete datasource; }, testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr(EXPECTED_MSG)));
}

TEST_F(DataSourceTest, CheckData) {

    SingleDataSource* datasource_B = make_single_datasource(
        DOUBLE_VALUE_DATA.c_str(),
        CREDENTIALS.c_str()
    );

    SurfaceBoundedSubVolume* subvolume_B = make_subvolume(
        datasource_B->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource_B, *subvolume_B, NEAREST, 0, size);
    int compared_values = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs_A = subvolume_A->vertical_segment(i);
        RawSegment rs_B = subvolume_B->vertical_segment(i);

        for (auto a_it = rs_A.begin(), b_it = rs_B.begin();
             a_it != rs_A.end() && b_it != rs_B.end();
             ++a_it, ++b_it) {
            compared_values++;
            EXPECT_NEAR(*a_it, *b_it / 2, DELTA);
        }
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);
    delete subvolume_B;
    delete datasource_B;
}

TEST_F(DataSourceTest, Addition) {

    DoubleDataSource* datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DOUBLE_VALUE_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_ref = subvolume_reference->vertical_segment(i);

        for (auto it = rs.begin(), a_it = rs_ref.begin();
             it != rs.end() && a_it != rs_ref.end();
             ++it, ++a_it) {
            compared_values++;
            EXPECT_NEAR(*it, *a_it * 3, DELTA) << "at segment " << i << " at position in data " << std::distance(rs.begin(), it);
        }
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);

    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, Multiplication) {

    DoubleDataSource* datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DOUBLE_VALUE_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_multiplication
    );

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);

    int compared_values = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_ref = subvolume_reference->vertical_segment(i);

        for (auto it = rs.begin(), a_it = rs_ref.begin();
             it != rs.end() && a_it != rs_ref.end();
             ++it, ++a_it) {
            compared_values++;
            EXPECT_NEAR(*it, 2 * (*a_it) * (*a_it), DELTA) << "at segment " << i << " at position in data " << std::distance(rs.begin(), it);
        }
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);
    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, Division) {

    DoubleDataSource* datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DOUBLE_VALUE_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_division
    );

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_ref = subvolume_reference->vertical_segment(i);

        for (auto it = rs.begin(), a_it = rs_ref.begin();
             it != rs.end() && a_it != rs_ref.end();
             ++it, ++a_it) {
            compared_values++;
            EXPECT_NEAR(*it, 0.5, DELTA) << "at segment " << i << " at position in data " << std::distance(rs.begin(), it);
        }
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);
    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, Subtraction) {

    DoubleDataSource* datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DOUBLE_VALUE_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_subtraction
    );

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_ref = subvolume_reference->vertical_segment(i);

        for (auto it = rs.begin(), a_it = rs_ref.begin();
             it != rs.end() && a_it != rs_ref.end();
             ++it, ++a_it) {
            compared_values++;
            EXPECT_NEAR(*it, -*a_it, DELTA) << "at segment " << i << " at position in data " << std::distance(rs.begin(), it);
        }
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);
    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, SubtractionReverse) {

    DoubleDataSource* datasource = make_double_datasource(
        DOUBLE_VALUE_DATA.c_str(),
        CREDENTIALS.c_str(),
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_subtraction
    );

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_ref = subvolume_reference->vertical_segment(i);

        for (auto it = rs.begin(), a_it = rs_ref.begin();
             it != rs.end() && a_it != rs_ref.end();
             ++it, ++a_it) {
            compared_values++;
            EXPECT_NEAR(*it, *a_it, DELTA) << "at segment " << i << " at position in data " << std::distance(rs.begin(), it);
        }
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);
    delete subvolume;
    delete datasource;
}

} // namespace
