#include "cppapi.hpp"
#include "ctypes.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const std::string DEFAULT_DATA = "file://dimensions_data_il_10_xl_20_s_30.vds";
const std::string DEFAULT_DATA_IL = "file://dimensions_data_il_11_xl_20_s_30.vds";
const std::string DEFAULT_DATA_XL = "file://dimensions_data_il_10_xl_21_s_30.vds";
const std::string DEFAULT_DATA_S = "file://dimensions_data_il_10_xl_20_s_31.vds";
const std::string DEFAULT_DATA_OS = "file://dimensions_data_il_10_xl_20_s_30_offset.vds";
const std::string DEFAULT_DATA_SS = "file://dimensions_data_il_10_xl_20_s_30_stepsize.vds";
const std::string DEFAULT_DATA_2X = "file://dimensions_data_il_10_xl_20_s_30_double.vds";

const std::string CREDENTIALS = "";

Grid default_grid = Grid(22, 33, 2, 3, 0);

class DataSourceTest : public ::testing::Test {
protected:
    void SetUp() override {
        datasource_A = make_single_datasource(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str());

        datasource_B = make_single_datasource(
            DEFAULT_DATA_2X.c_str(),
            CREDENTIALS.c_str());

        for (int i = 0; i < size; ++i) {
            top_surface_data[i] = 19.0;
            primary_surface_data[i] = 20.0;
            bottom_surface_data[i] = 29.0;
        };

        subvolume_A = make_subvolume(
            datasource_A->get_metadata(), primary_surface, top_surface, bottom_surface);

        subvolume_B = make_subvolume(
            datasource_B->get_metadata(), primary_surface, top_surface, bottom_surface);

        cppapi::fetch_subvolume(*datasource_A, *subvolume_A, NEAREST, 0, size);
        cppapi::fetch_subvolume(*datasource_B, *subvolume_B, NEAREST, 0, size);
    }

    void TearDown() override {
        delete subvolume_A;
        delete subvolume_B;
        delete datasource_A;
        delete datasource_B;
    }

    DoubleDataSource *datasource;
    SingleDataSource *datasource_A;
    SingleDataSource *datasource_B;
    SurfaceBoundedSubVolume *subvolume_A;
    SurfaceBoundedSubVolume *subvolume_B;
    static constexpr int nrows = 10;
    static constexpr int ncols = 20;
    static constexpr std::size_t size = nrows * ncols;

    std::array<float, size> top_surface_data;
    std::array<float, nrows *ncols> primary_surface_data;
    std::array<float, size> bottom_surface_data;

    static constexpr float fill = -999.25;

    RegularSurface primary_surface =
        RegularSurface(primary_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface top_surface =
        RegularSurface(top_surface_data.data(), nrows, ncols, default_grid, fill);

    RegularSurface bottom_surface =
        RegularSurface(bottom_surface_data.data(), nrows, ncols, default_grid, fill);
};

TEST_F(DataSourceTest, ILineMismatch) {
    const std::string EXPECTED_MSG = "Axis: Inline: Mismatch in number of samples: 10 != 11";
    try {
        DoubleDataSource *datasource = make_double_datasource(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str(),
            DEFAULT_DATA_IL.c_str(),
            CREDENTIALS.c_str(),
            &inplace_subtraction);
        delete datasource;
    } catch (std::runtime_error const &err) {
        EXPECT_EQ(err.what(), std::string(EXPECTED_MSG));
    } catch (...) {
        FAIL() << "Expected:" << EXPECTED_MSG;
    }
}

TEST_F(DataSourceTest, XLineMismatch) {
    const std::string EXPECTED_MSG = "Axis: Crossline: Mismatch in number of samples: 20 != 21";
    try {
        DoubleDataSource *datasource = make_double_datasource(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str(),
            DEFAULT_DATA_XL.c_str(),
            CREDENTIALS.c_str(),
            &inplace_subtraction);
        delete datasource;
    } catch (std::runtime_error const &err) {
        EXPECT_EQ(err.what(), std::string(EXPECTED_MSG));
    } catch (...) {
        FAIL() << "Expected:" << EXPECTED_MSG;
    }
}

TEST_F(DataSourceTest, SamplesMismatch) {
    const std::string EXPECTED_MSG = "Axis: Sample: Mismatch in number of samples: 30 != 31";
    try {
        DoubleDataSource *datasource = make_double_datasource(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str(),
            DEFAULT_DATA_S.c_str(),
            CREDENTIALS.c_str(),
            &inplace_subtraction);
        delete datasource;
    } catch (std::runtime_error const &err) {
        EXPECT_EQ(err.what(), std::string(EXPECTED_MSG));
    } catch (...) {
        FAIL() << "Expected:" << EXPECTED_MSG;
    }
}

TEST_F(DataSourceTest, OffsetMismatch) {
    const std::string EXPECTED_MSG = "Axis: Inline: Mismatch in min value: 22.000000 != 25.000000";
    try {
        DoubleDataSource *datasource = make_double_datasource(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str(),
            DEFAULT_DATA_OS.c_str(),
            CREDENTIALS.c_str(),
            &inplace_subtraction);
        delete datasource;
    } catch (std::runtime_error const &err) {
        EXPECT_EQ(err.what(), std::string(EXPECTED_MSG));
    } catch (...) {
        FAIL() << "Expected:" << EXPECTED_MSG;
    }
}

TEST_F(DataSourceTest, StepSizeMismatch) {
    // Changing the stepsize causes the max value to change. Max value is checked before stepsize.
    const std::string EXPECTED_MSG = "Axis: Inline: Mismatch in max value: 40.000000 != 139.000000";
    try {
        DoubleDataSource *datasource = make_double_datasource(
            DEFAULT_DATA.c_str(),
            CREDENTIALS.c_str(),
            DEFAULT_DATA_SS.c_str(),
            CREDENTIALS.c_str(),
            &inplace_subtraction);
        delete datasource;
    } catch (std::runtime_error const &err) {
        EXPECT_EQ(err.what(), std::string(EXPECTED_MSG));
    } catch (...) {
        FAIL() << "Expected:" << EXPECTED_MSG;
    }
}

TEST_F(DataSourceTest, Addition) {

    DoubleDataSource *datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DEFAULT_DATA_2X.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition);

    SurfaceBoundedSubVolume *subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface);

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    int compared_expected = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_A = subvolume_A->vertical_segment(i);
        RawSegment rs_B = subvolume_B->vertical_segment(i);

        std::vector<float>::const_iterator it;
        std::size_t offset = 0;
        compared_expected += rs.size();
        for (it = rs.begin(); it != rs.end(); it++) {
            offset = it - rs.begin();
            float target_value = *(rs.begin() + offset);
            float value_A = *(rs_A.begin() + offset);
            float value_B = *(rs_B.begin() + offset);
            compared_values++;
            EXPECT_EQ(target_value, value_A + value_B) << "Expected value: " << value_A + value_B << " Actual value: " << target_value;
        }
    }
    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, Multiplication) {

    DoubleDataSource *datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DEFAULT_DATA_2X.c_str(),
        CREDENTIALS.c_str(),
        &inplace_multiplication);

    SurfaceBoundedSubVolume *subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface);

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);

    int compared_values = 0;
    int compared_expected = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_A = subvolume_A->vertical_segment(i);
        RawSegment rs_B = subvolume_B->vertical_segment(i);

        std::vector<float>::const_iterator it;
        std::size_t offset = 0;
        compared_expected += rs.size();
        for (it = rs.begin(); it != rs.end(); it++) {
            offset = it - rs.begin();
            float target_value = *(rs.begin() + offset);
            float value_A = *(rs_A.begin() + offset);
            float value_B = *(rs_B.begin() + offset);
            compared_values++;            
            EXPECT_EQ(target_value, value_A * value_B) << "Expected value: " << value_A * value_B << " Actual value: " << target_value;
        }
    }
    EXPECT_EQ(compared_values, compared_expected) << "Compared values: " << compared_values << " Expected values: " << compared_expected;
    EXPECT_GE(compared_values, size) << "Compared values: " << compared_values << " Expected at least one value per trace ";
    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, Division) {

    DoubleDataSource *datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DEFAULT_DATA_2X.c_str(),
        CREDENTIALS.c_str(),
        &inplace_division);

    SurfaceBoundedSubVolume *subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface);

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    int compared_expected = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_A = subvolume_A->vertical_segment(i);
        RawSegment rs_B = subvolume_B->vertical_segment(i);

        std::vector<float>::const_iterator it;
        std::size_t offset = 0;
        compared_expected += rs.size();
        for (it = rs.begin(); it != rs.end(); it++) {
            offset = it - rs.begin();
            float target_value = *(rs.begin() + offset);
            float value_A = *(rs_A.begin() + offset);
            float value_B = *(rs_B.begin() + offset);
            compared_values++;
            EXPECT_EQ(target_value, value_A / value_B) << "Expected value: " << value_A / value_B << " Actual value: " << target_value;
        }
    }
    EXPECT_EQ(compared_values, compared_expected) << "Compared values: " << compared_values << " Expected values: " << compared_expected;
    EXPECT_GE(compared_values, size) << "Compared values: " << compared_values << " Expected at least one value per trace ";
    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, Subtract) {

    DoubleDataSource *datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        DEFAULT_DATA_2X.c_str(),
        CREDENTIALS.c_str(),
        &inplace_subtraction);

    SurfaceBoundedSubVolume *subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface);

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    int compared_expected = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_A = subvolume_A->vertical_segment(i);
        RawSegment rs_B = subvolume_B->vertical_segment(i);

        std::vector<float>::const_iterator it;
        std::size_t offset = 0;
        compared_expected += rs.size();
        for (it = rs.begin(); it != rs.end(); it++) {
            offset = it - rs.begin();
            float target_value = *(rs.begin() + offset);
            float value_A = *(rs_A.begin() + offset);
            float value_B = *(rs_B.begin() + offset);
            compared_values++;
            EXPECT_EQ(target_value, value_A - value_B) << "Expected value: " << value_A - value_B << " Actual value: " << target_value;
        }
    }
    EXPECT_EQ(compared_values, compared_expected) << "Compared values: " << compared_values << " Expected values: " << compared_expected;
    EXPECT_GE(compared_values, size) << "Compared values: " << compared_values << " Expected at least one value per trace ";
    delete subvolume;
    delete datasource;
}

TEST_F(DataSourceTest, SubtractReverse) {

    DoubleDataSource *datasource = make_double_datasource(
        DEFAULT_DATA_2X.c_str(),
        CREDENTIALS.c_str(),
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_subtraction);

    SurfaceBoundedSubVolume *subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface);

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    int compared_expected = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);

        // Switch the order of the subvolumes
        RawSegment rs_A = subvolume_B->vertical_segment(i);
        RawSegment rs_B = subvolume_A->vertical_segment(i);

        std::vector<float>::const_iterator it;
        std::size_t offset = 0;
        compared_expected += rs.size();
        for (it = rs.begin(); it != rs.end(); it++) {
            offset = it - rs.begin();
            float target_value = *(rs.begin() + offset);
            float value_A = *(rs_A.begin() + offset);
            float value_B = *(rs_B.begin() + offset);
            compared_values++;
            EXPECT_EQ(target_value, value_A - value_B) << "Expected value: " << value_A - value_B << " Actual value: " << target_value << " A=" << value_A << " B=" << value_B;
        }
    }
    EXPECT_EQ(compared_values, compared_expected) << "Compared values: " << compared_values << " Expected values: " << compared_expected;
    EXPECT_GE(compared_values, size) << "Compared values: " << compared_values << " Expected at least one value per trace ";
    delete subvolume;
    delete datasource;
}

} // namespace