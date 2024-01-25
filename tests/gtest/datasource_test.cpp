#include "cppapi.hpp"
#include "ctypes.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const std::string DEFAULT_DATA = "file://10_samples_default.vds";
const std::string DOUBLE_VALUE_DATA = "file://10_double_value.vds";
const std::string PARTIAL_INTERSECT_DATA = "file://10_offset_value.vds";
const std::string NO_INTERSECT_DATA = "file://10_no_intersection.vds";


const std::string CREDENTIALS = "";

const float DELTA = 0.00001;
const int EXPECTED_TRACE_LENGTH = 7;

const float x_inc = std::sqrt((8 - 2) * (8 - 2) + 4 * 4);                                            // 7.2111
const float y_inc = std::sqrt((-2) * (-2) + 3 * 3);                                                  // 3.6056
const float angle = std::asin(4 / std::sqrt((8 - 2) * (8 - 2) + 4 * 4)) / (2 * std::acos(-1)) * 360; // 33.69
Grid default_grid = Grid(2, 0, x_inc, y_inc, angle);

class DataSourceTest : public ::testing::Test {
protected:
    DoubleDataSource* datasource;
    SingleDataSource* datasource_reference;
    SurfaceBoundedSubVolume* subvolume_reference;
    static constexpr int nrows = 3;
    static constexpr int ncols = 2;
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

TEST_F(DataSourceTest, No_Intersect) {
    // Changing the stepsize causes the max value to change. Max value is checked before stepsize.
    const std::string EXPECTED_MSG = "Axis: Crossline: No data overlap: min = 12.000000 and max = 11.000000";

    EXPECT_THAT([&]() {         
            DoubleDataSource *datasource = make_double_datasource(
                DEFAULT_DATA.c_str(),
                CREDENTIALS.c_str(),
                NO_INTERSECT_DATA.c_str(),
                CREDENTIALS.c_str(),
                &inplace_subtraction);
                delete datasource; }, testing::ThrowsMessage<std::runtime_error>(testing::HasSubstr(EXPECTED_MSG)));
}


TEST_F(DataSourceTest, Intersect) {

    DoubleDataSource* datasource = make_double_datasource(
        DEFAULT_DATA.c_str(),
        CREDENTIALS.c_str(),
        PARTIAL_INTERSECT_DATA.c_str(),
        CREDENTIALS.c_str(),
        &inplace_addition
    );

    SurfaceBoundedSubVolume* subvolume = make_subvolume(
        datasource->get_metadata(), primary_surface, top_surface, bottom_surface
    );

    // std::cout << "Test ()" <<  std::endl;
    // std::cout << "Test" <<  datasource->get_metadata().iline().min() << std::endl;

    // Print data to standard out

    cppapi::fetch_subvolume(*datasource, *subvolume, NEAREST, 0, size);
    int compared_values = 0;
    for (int i = 0; i < size; ++i) {
        RawSegment rs = subvolume->vertical_segment(i);
        RawSegment rs_ref = subvolume_reference->vertical_segment(i);

        std::cout << rs.size() << " " << rs_ref.size() << std::endl;

        for (auto it = rs.begin(); it != rs.end(); ++it) {
            std::cout << i << " " << *it << std::endl;
        }

        for (auto it = rs.begin(), a_it = rs_ref.begin();
             it != rs.end() && a_it != rs_ref.end();
             ++it, ++a_it) {
            compared_values++;
            std::cout << "*it: " << *it << " *a_it: " << *a_it << " i: " << i << "  " <<   std::distance(rs.begin(), it) << std::endl;
            EXPECT_NEAR(*it, *a_it * 3, DELTA) << "at segment " << i << " at position in data " << std::distance(rs.begin(), it);
        }
        std::cout << std::endl;
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);

    delete subvolume;
    delete datasource;
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
    // std::cout << "\n\nStart test  SubtractionReverse" << std::endl;
    // std::cout << DOUBLE_VALUE_DATA.c_str() << std::endl;
    // std::cout << DEFAULT_DATA.c_str() << std::endl;

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
        for (auto it = rs.begin(), a_it = rs_ref.begin(); it != rs.end() && a_it != rs_ref.end(); ++it, ++a_it) {
            compared_values++;
            EXPECT_NEAR(*it, *a_it, DELTA) << "at segment " << i << " at position in data " << std::distance(rs.begin(), it);
        }
    }
    EXPECT_EQ(compared_values, size * EXPECTED_TRACE_LENGTH);
    delete subvolume;
    delete datasource;
}

} // namespace
