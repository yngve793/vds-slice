#include "cppapi.hpp"
#include "ctypes.h"
#include <iostream>
#include <sstream>

#include "attribute.hpp"
#include "metadatahandle.hpp"
#include "subvolume.hpp"
#include "utils.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const std::string REGULAR_DATA = "file://regular_8x3_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x3_cube.vds";
const std::string SHIFT_8_BIG_DATA = "file://big_shift_8_32x3_cube.vds";

const std::string CREDENTIALS = "";

double norm(const OpenVDS::Vector<double, 4UL>& vec) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += std::pow(vec[i], 2);
    }
    return std::sqrt(sum);
}

class DatahandleAttributeTest : public ::testing::Test {

    void SetUp() override {

        iline_array = std::vector<int>(32);
        xline_array = std::vector<int>(32);
        sample_array = std::vector<int>(32);

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

    static constexpr float fill = -999.25;

    Grid get_grid(DataHandle* datahandle) {

        const MetadataHandle* metadata = &(datahandle->get_metadata());
        const OpenVDS::DoubleMatrix4x4 ijkToWorldTransform = metadata->coordinate_transformer().IJKToWorldTransform();

        double xori = ijkToWorldTransform.data[3][0];
        double yori = ijkToWorldTransform.data[3][1];
        double xinc = norm(ijkToWorldTransform.data[0]);
        double yinc = norm(ijkToWorldTransform.data[1]);
        double rot = std::atan(ijkToWorldTransform.data[0][1] / ijkToWorldTransform.data[0][0]) / M_PI * 180;

        return Grid(xori, yori, xinc, yinc, rot);
    }

    void check_attribute(SurfaceBoundedSubVolume& subvolume, int low[], int high[], float factor) {

        std::size_t nr_of_values = subvolume.nsamples(0, (high[0] - low[0]) * (high[1] - low[1]));
        EXPECT_EQ(nr_of_values, (high[0] - low[0]) * (high[1] - low[1]) * (high[2] - low[2]));

        int counter = 0;
        for (int il = low[0]; il < high[0]; ++il) {
            for (int xl = low[1]; xl < high[1]; ++xl) {
                RawSegment rs = subvolume.vertical_segment(counter);
                int s = low[2];
                for (auto it = rs.begin(); it != rs.end(); ++it) {
                    int value = int(*it / factor + 0.5f);
                    int sample = value & 0xFF;
                    int xline = (value & 0xFF00) >> 8;
                    int iline = (value & 0xFF0000) >> 16;

                    EXPECT_EQ(iline, iline_array[il]);
                    EXPECT_EQ(xline, xline_array[xl]);
                    EXPECT_EQ(sample, sample_array[s]);
                    s += 1;
                }
                EXPECT_EQ(s, high[2]);
                counter += 1;
            }
        }
    }
};

TEST_F(DatahandleAttributeTest, Attribute_Single) {

    DataHandle* datahandle = single_datahandle;
    Grid grid = get_grid(datahandle);
    const MetadataHandle* metadata = &(datahandle->get_metadata());

    std::size_t nrows = metadata->iline().nsamples();
    std::size_t ncols = metadata->xline().nsamples();
    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> pri_surface_data(nrows * ncols, 36.0f);
    static std::vector<float> bot_surface_data(nrows * ncols, 52.0f);
    RegularSurface pri_surface = RegularSurface(pri_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface top_surface = RegularSurface(top_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface bot_surface = RegularSurface(bot_surface_data.data(), nrows, ncols, grid, fill);
    SurfaceBoundedSubVolume* subvolume = make_subvolume(datahandle->get_metadata(), pri_surface, top_surface, bot_surface);

    cppapi::fetch_subvolume(*single_datahandle, *subvolume, NEAREST, 0, nrows * ncols);

    int low[3] = {0, 0, 4};
    int high[3] = {8, 8, 15};
    check_attribute(*subvolume, low, high, 1);

    delete subvolume;
}

TEST_F(DatahandleAttributeTest, Attribute_Double) {

    DataHandle* datahandle = double_datahandle;
    Grid grid = get_grid(datahandle);
    const MetadataHandle* metadata = &(datahandle->get_metadata());

    std::size_t nrows = metadata->iline().nsamples();
    std::size_t ncols = metadata->xline().nsamples();
    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> pri_surface_data(nrows * ncols, 36.0f);
    static std::vector<float> bot_surface_data(nrows * ncols, 52.0f);
    RegularSurface pri_surface = RegularSurface(pri_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface top_surface = RegularSurface(top_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface bot_surface = RegularSurface(bot_surface_data.data(), nrows, ncols, grid, fill);
    SurfaceBoundedSubVolume* subvolume = make_subvolume(datahandle->get_metadata(), pri_surface, top_surface, bot_surface);

    cppapi::fetch_subvolume(*datahandle, *subvolume, NEAREST, 0, nrows * ncols);

    int low[3] = {4, 4, 4};
    int high[3] = {8, 8, 15};
    check_attribute(*subvolume, low, high, 2);

    delete subvolume;
}

TEST_F(DatahandleAttributeTest, Attribute_Reverse_Double) {

    DataHandle* datahandle = double_reverse_datahandle;
    Grid grid = get_grid(datahandle);
    const MetadataHandle* metadata = &(datahandle->get_metadata());

    std::size_t nrows = metadata->iline().nsamples();
    std::size_t ncols = metadata->xline().nsamples();
    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> pri_surface_data(nrows * ncols, 36.0f);
    static std::vector<float> bot_surface_data(nrows * ncols, 52.0f);
    RegularSurface pri_surface = RegularSurface(pri_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface top_surface = RegularSurface(top_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface bot_surface = RegularSurface(bot_surface_data.data(), nrows, ncols, grid, fill);
    SurfaceBoundedSubVolume* subvolume = make_subvolume(datahandle->get_metadata(), pri_surface, top_surface, bot_surface);

    cppapi::fetch_subvolume(*datahandle, *subvolume, NEAREST, 0, nrows * ncols);

    int low[3] = {4, 4, 4};
    int high[3] = {8, 8, 15};
    check_attribute(*subvolume, low, high, 2);

    delete subvolume;
}

TEST_F(DatahandleAttributeTest, Attribute_Empty_Double) {

    DataHandle* datahandle = double_empty;
    Grid grid = get_grid(datahandle);
    const MetadataHandle* metadata = &(datahandle->get_metadata());

    std::size_t nrows = metadata->iline().nsamples();
    std::size_t ncols = metadata->xline().nsamples();
    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> pri_surface_data(nrows * ncols, 36.0f);
    static std::vector<float> bot_surface_data(nrows * ncols, 52.0f);
    RegularSurface pri_surface = RegularSurface(pri_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface top_surface = RegularSurface(top_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface bot_surface = RegularSurface(bot_surface_data.data(), nrows, ncols, grid, fill);
    SurfaceBoundedSubVolume* subvolume = make_subvolume(datahandle->get_metadata(), pri_surface, top_surface, bot_surface);

    cppapi::fetch_subvolume(*datahandle, *subvolume, NEAREST, 0, nrows * ncols);

    int low[3] = {8, 8, 4};
    int high[3] = {8, 8, 4};
    check_attribute(*subvolume, low, high, 2);

    delete subvolume;
}

TEST_F(DatahandleAttributeTest, Attribute_Different_Size_Double) {

    DataHandle* datahandle = double_different_size;
    Grid grid = get_grid(datahandle);
    const MetadataHandle* metadata = &(datahandle->get_metadata());

    std::size_t nrows = metadata->iline().nsamples();
    std::size_t ncols = metadata->xline().nsamples();
    static std::vector<float> top_surface_data(nrows * ncols, 44.0f);
    static std::vector<float> pri_surface_data(nrows * ncols, 52.0f);
    static std::vector<float> bot_surface_data(nrows * ncols, 72.0f);
    RegularSurface pri_surface = RegularSurface(pri_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface top_surface = RegularSurface(top_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface bot_surface = RegularSurface(bot_surface_data.data(), nrows, ncols, grid, fill);
    SurfaceBoundedSubVolume* subvolume = make_subvolume(datahandle->get_metadata(), pri_surface, top_surface, bot_surface);

    cppapi::fetch_subvolume(*datahandle, *subvolume, NEAREST, 0, nrows * ncols);

    int low[3] = {8, 8, 8};
    int high[3] = {12, 12, 20};
    check_attribute(*subvolume, low, high, 2);

    delete subvolume;
}

} // namespace
