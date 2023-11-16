#include "cppapi.hpp"
#include "ctypes.h"

#include <gmock/gmock.h>

namespace {

const std::string SAMPLES_10 = "file://10_samples_default.vds";

const std::string CREDENTIALS = "";

Grid samples_10_grid = Grid(2, 0, 7.2111, 3.6056, 33.69);

class SubCubeTest : public ::testing::Test {
protected:
    void SetUp() override {
        datasource_A = make_single_datasource(
            SAMPLES_10.c_str(),
            CREDENTIALS.c_str());

        subvolume_A = make_subvolume(
            datasource_A->get_metadata(), primary_surface, top_surface, bottom_surface);

        cppapi::fetch_subvolume(*datasource_A, *subvolume_A, NEAREST, 0, size);
    }

    void TearDown() override {
        delete subvolume_A;
        delete datasource_A;
    }

    SingleDataSource *datasource_A;
    SurfaceBoundedSubVolume *subvolume_A;
    static constexpr int nrows = 3;
    static constexpr int ncols = 2;
    static constexpr std::size_t size = nrows * ncols;

    std::array<float, size> top_surface_data = {16, 16, 16, 16, 16, 16};
    std::array<float, nrows *ncols> primary_surface_data = {20, 20, 20, 20, 20, 20};
    std::array<float, size> bottom_surface_data = {24, 24, 24, 24, 24, 24};

    static constexpr float fill = -999.25;

    RegularSurface primary_surface =
        RegularSurface(primary_surface_data.data(), nrows, ncols, samples_10_grid, fill);

    RegularSurface top_surface =
        RegularSurface(top_surface_data.data(), nrows, ncols, samples_10_grid, fill);

    RegularSurface bottom_surface =
        RegularSurface(bottom_surface_data.data(), nrows, ncols, samples_10_grid, fill);
};


} // namespace
