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

const std::string CREDENTIALS = "";

double norm(const std::vector<double>& vec) {
    double sum = 0.0;
    for (double component : vec) {
        sum += std::pow(component, 2);
    }
    return std::sqrt(sum);
}

template< typename T >
void append(std::vector< std::unique_ptr< AttributeMap > >& vec, T obj) {
    vec.push_back( std::unique_ptr< T >( new T( std::move(obj) ) ) );
}

class PhaseTest : public ::testing::Test {

    void SetUp() override {

        iline_array = std::vector<int>(8);
        xline_array = std::vector<int>(8);
        sample_array = std::vector<int>(32);

        for (int i = 0; i < iline_array.size(); i++) {
            iline_array[i] = 3 + i * 3;
            xline_array[i] = 2 + i * 2;
        }

        for (int i = 0; i < sample_array.size(); i++) {
            sample_array[i] = 4 + i * 4;
        }

        single_datasource = make_single_datasource(
            REGULAR_DATA.c_str(),
            CREDENTIALS.c_str()
        );
    }

    void TearDown() override {
        delete single_datasource;
        delete double_datasource;
        delete double_reverse_datasource;
    }

public:
    nlohmann::json expected_intersect_metadata;

    std::vector<int> iline_array;
    std::vector<int> xline_array;
    std::vector<int> sample_array;

    std::vector<Bound> slice_bounds;

    SingleDataSource* single_datasource;
    DoubleDataSource* double_datasource;
    DoubleDataSource* double_reverse_datasource;

    static constexpr float fill = -999.25;

    // void check_attribute(SurfaceBoundedSubVolume& subvolume, int low[], int high[], float factor) {

    //     std::size_t nr_of_values = subvolume.nsamples(0, (high[0] - low[0]) * (high[1] - low[1]));
    //     EXPECT_EQ(nr_of_values, (high[0] - low[0]) * (high[1] - low[1]) * (high[2] - low[2]));

    //     int counter = 0;
    //     for (int il = low[0]; il < high[0]; ++il) {
    //         for (int xl = low[1]; xl < high[1]; ++xl) {
    //             RawSegment rs = subvolume.vertical_segment(counter);
    //             int s = low[2];
    //             for (auto it = rs.begin(); it != rs.end(); ++it) {
    //                 int value = int(*it / factor + 0.5f);
    //                 int sample = value & 0xFF;
    //                 int xline = (value & 0xFF00) >> 8;
    //                 int iline = (value & 0xFF0000) >> 16;

    //                 EXPECT_EQ(iline, iline_array[il]);
    //                 EXPECT_EQ(xline, xline_array[xl]);
    //                 EXPECT_EQ(sample, sample_array[s]);
    //                 s += 1;
    //             }
    //             EXPECT_EQ(s, high[2]);
    //             counter += 1;
    //         }
    //     }
    // }
};

TEST_F(PhaseTest, Single_Attribute) {

    DataSource* datasource = single_datasource;
    // Grid grid = get_grid(datasource);
    Grid grid = Grid(2, 0, 7.2111, 3.6056, 33.69);
    const MetadataHandle* metadata = &(datasource->get_metadata());

    std::size_t nrows = metadata->iline().nsamples();
    std::size_t ncols = metadata->xline().nsamples();
    static std::vector<float> top_surface_data(nrows * ncols, 28.0f);
    static std::vector<float> pri_surface_data(nrows * ncols, 44.0f);
    static std::vector<float> bot_surface_data(nrows * ncols, 52.0f);
    RegularSurface pri_surface = RegularSurface(pri_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface top_surface = RegularSurface(top_surface_data.data(), nrows, ncols, grid, fill);
    RegularSurface bot_surface = RegularSurface(bot_surface_data.data(), nrows, ncols, grid, fill);
    SurfaceBoundedSubVolume* subvolume = make_subvolume(datasource->get_metadata(), pri_surface, top_surface, bot_surface);

    cppapi::fetch_subvolume(*single_datasource, *subvolume, NEAREST, 0, nrows * ncols);
    ResampledSegmentBlueprint dst_segment_blueprint = ResampledSegmentBlueprint(4.0f);

    std::size_t size = subvolume->horizontal_grid().size() * sizeof(float);

    std::vector<float> values(subvolume->horizontal_grid().size());

    std::vector<std::unique_ptr<AttributeMap>> attrs;
    append(attrs,   Phase((void*)(values.data()), size)     );

    calc_attributes(*subvolume, &dst_segment_blueprint, attrs, 0, nrows* ncols);

    delete subvolume;
}

} // namespace