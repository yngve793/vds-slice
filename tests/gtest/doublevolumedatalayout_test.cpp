#include "cppapi.hpp"
#include "ctypes.h"
#include <iostream>
#include <sstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const std::string REGULAR_DATA = "file://regular_8x3_cube.vds";
const std::string SHIFT_4_DATA = "file://shift_4_8x3_cube.vds";
const std::string SHIFT_M4_DATA = "file://shift_minus4_8x3_cube.vds";

const std::string CREDENTIALS = "";

class DoubleVolumeDataLayoutTest : public ::testing::Test {
protected:
    DoubleDataSource* datasource;

    void SetUp() override {}

    void TearDown() override {}
};

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

    // delete subvolume;
    delete datasource;
}

} // namespace
