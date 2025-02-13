// #include "cppapi.hpp"
// #include "ctypes.h"
// #include <iostream>
// #include <sstream>

// #include "attribute.hpp"
// #include "metadatahandle.hpp"
// #include "nlohmann/json.hpp"
// #include "subvolume.hpp"
// #include "utils.hpp"
// #include "gmock/gmock.h"
// #include "gtest/gtest.h"

// #include <fstream>
// using json = nlohmann::json;

// namespace {
// const std::string Volve_150_bw = "file://ST0202ZDC12-PZ-PSDM-KIRCH-FULL-T.MIG_FIN.POST_STACK.3D.JS-017534 [BW].json";
// const std::string Volve_150_freq = "file://ST0202ZDC12-PZ-PSDM-KIRCH-FULL-T.MIG_FIN.POST_STACK.3D.JS-017534 [Freq].json";
// const std::string Volve_150_phase = "file://ST0202ZDC12-PZ-PSDM-KIRCH-FULL-T.MIG_FIN.POST_STACK.3D.JS-017534 [Phase].json";
// const std::string Volve_150_sweet = "file://ST0202ZDC12-PZ-PSDM-KIRCH-FULL-T.MIG_FIN.POST_STACK.3D.JS-017534 [Sweet].json";
// const std::string Volve_150 = "file://ST0202ZDC12-PZ-PSDM-KIRCH-FULL-T.MIG_FIN.POST_STACK.3D.JS-017534.json";

// const std::string CREDENTIALS = "";

// double norm(const std::vector<double>& vec) {
//     double sum = 0.0;
//     for (double component : vec) {
//         sum += std::pow(component, 2);
//     }
//     return std::sqrt(sum);
// }

// template <typename T>
// void append(std::vector<std::unique_ptr<AttributeMap>>& vec, T obj) {
//     vec.push_back(std::unique_ptr<T>(new T(std::move(obj))));
// }

// class AttributeHilbertExtendedTest : public ::testing::Test {

//     void SetUp() override {

//         volve_150 = make_single_datahandle(
//             Volve_150.c_str(),
//             CREDENTIALS.c_str()
//         );

//         volve_150_phase = make_single_datahandle(
//             Volve_150_phase.c_str(),
//             CREDENTIALS.c_str()
//         );
//     }

//     void TearDown() override {
//         delete volve_150;
//         delete volve_150_phase;
//     }

// public:
//     SingleDataHandle* volve_150;
//     SingleDataHandle* volve_150_phase;
//     static constexpr float fill = -999.25;
// };

// TEST_F(AttributeHilbertExtendedTest, Phase_Index_K_Slice_2) {

//     std::vector<Bound> slice_bounds;
//     struct response response_data_a;
//     cppapi::slice(
//         *penobscot,
//         Direction(axis_name::K),
//         300,
//         slice_bounds,
//         &response_data_a
//     );

//     auto response_samples_a = (float*)response_data_a.data;
//     EXPECT_EQ(int(response_data_a.size / 4), 601 * 2);

//     for (int i = 0; i < int(response_data_a.size / 8); i++) {
//         EXPECT_EQ(response_samples_a[i * 2], response_samples_a[i * 2 + 1]);
//     }
// }

// TEST_F(AttributeHilbertExtendedTest, Phase_Annotated_Sample_Slice_2) {

//     std::vector<Bound> slice_bounds;
//     struct response response_data_a;
//     cppapi::slice(
//         *penobscot,
//         Direction(axis_name::SAMPLE),
//         2400,
//         slice_bounds,
//         &response_data_a
//     );

//     auto response_samples_a = (float*)response_data_a.data;
//     EXPECT_EQ(int(response_data_a.size / 4), 601 * 2);

//     for (int i = 0; i < int(response_data_a.size / 8); i++) {
//         EXPECT_EQ(response_samples_a[i * 2], response_samples_a[i * 2 + 1]);
//     }

//     for (int i = 0; i < int(response_data_a.size / 8); i++) {
//         EXPECT_EQ(response_samples_a[i * 2], response_samples_a[i * 2 + 1]);
//     }
// }

// TEST_F(AttributeHilbertExtendedTest, Phase_Index_J_Slice_2) {

//     std::vector<Bound> slice_bounds;
//     struct response response_data_a;
//     cppapi::slice(
//         *penobscot,
//         Direction(axis_name::J),
//         0,
//         slice_bounds,
//         &response_data_a
//     );

//     struct response response_data_b;
//     cppapi::slice(
//         *penobscot,
//         Direction(axis_name::J),
//         1,
//         slice_bounds,
//         &response_data_b
//     );

//     EXPECT_EQ(int(response_data_a.size / 4), 601 * 1501);
//     EXPECT_EQ(int(response_data_b.size / 4), 601 * 1501);

//     auto response_samples_a = (float*)response_data_a.data;
//     auto response_samples_b = (float*)response_data_b.data;

//     for (int i = 0; i < int(response_data_a.size / 4); i++) {
//         EXPECT_EQ(response_samples_a[i], response_samples_b[i]);
//     }
// }

// TEST_F(AttributeHilbertExtendedTest, Phase_Attribute_2_2) {

//     DataHandle* datahandle = penobscot;
//     Grid grid = Grid(2, 0, 3.6056, 3.6056, 33.69);

//     const MetadataHandle* metadata = &(datahandle->get_metadata());

//     std::size_t nrows = metadata->iline().nsamples();
//     std::size_t ncols = metadata->xline().nsamples();
//     static std::vector<float> top_surface_data(nrows * ncols, 2000.0f);
//     static std::vector<float> pri_surface_data(nrows * ncols, 2400.0f);
//     static std::vector<float> bot_surface_data(nrows * ncols, 3000.0f);
//     RegularSurface pri_surface = RegularSurface(pri_surface_data.data(), nrows, ncols, grid, fill);
//     RegularSurface top_surface = RegularSurface(top_surface_data.data(), nrows, ncols, grid, fill);
//     RegularSurface bot_surface = RegularSurface(bot_surface_data.data(), nrows, ncols, grid, fill);
//     SurfaceBoundedSubVolume* subvolume = make_subvolume(datahandle->get_metadata(), pri_surface, top_surface, bot_surface);

//     cppapi::fetch_subvolume(*datahandle, *subvolume, NEAREST, 0, nrows * ncols);

//     ResampledSegmentBlueprint dst_segment_blueprint = ResampledSegmentBlueprint(4.0f);

//     std::vector<enum attribute> attributes(0);
//     attributes.push_back(attribute::VALUE);
//     attributes.push_back(attribute::PHASE);
//     attributes.push_back(attribute::ENVELOPE);
//     attributes.push_back(attribute::HILBERT);

//     std::vector<float> buffer(nrows * ncols * attributes.size());

//     // Point to the start of each attribute buffer
//     std::vector<void*> attribute_buffers(attributes.size());
//     for (int i = 0; i < attributes.size(); i++) {
//         attribute_buffers[i] = &(buffer[nrows * ncols * i]);
//     }

//     void* out_d = buffer.data();

//     cppapi::attributes(
//         *subvolume,
//         &dst_segment_blueprint,
//         attributes.data(),
//         attributes.size(),
//         0,
//         nrows * ncols,
//         attribute_buffers.data()
//     );

//     std::ifstream f(PENOBSCOT_EXPECTED.c_str());
//     json penobscot_expected = json::parse(f);

//     auto value_expected = penobscot_expected["value"].template get<std::vector<float>>();
//     EXPECT_EQ(nrows, value_expected.size());

//     float* values = (float*)attribute_buffers[0];
//     for (int i = 0; i < nrows; i++) {
//         EXPECT_EQ(values[i * 2], values[i * 2 + 1]) << "Unexpected value at index " << i;
//         EXPECT_EQ(values[i * 2], value_expected[i]) << "Unexpected value at index " << i;
//     }

//     auto phase_expected = penobscot_expected["phase"].template get<std::vector<float>>();
//     EXPECT_EQ(nrows, phase_expected.size());

//     float* phases = (float*)attribute_buffers[1];
//     for (int i = 0; i < nrows; i++) {
//         EXPECT_EQ(phases[i * 2], phases[i * 2 + 1]);
//         EXPECT_NEAR(phases[i * 2], phase_expected[i], 2e-14);
//     }

//     auto envelope_expected = penobscot_expected["envelope"].template get<std::vector<float>>();
//     EXPECT_EQ(nrows, envelope_expected.size());

//     float* envelope = (float*)attribute_buffers[2];
//     for (int i = 0; i < nrows; i++) {
//         EXPECT_EQ(envelope[i * 2], envelope[i * 2 + 1]);
//         EXPECT_NEAR(envelope[i * 2], envelope_expected[i], 2e-14);
//     }

//     auto hilbert_expected = penobscot_expected["hilbert"].template get<std::vector<float>>();
//     EXPECT_EQ(nrows, hilbert_expected.size());

//     float* hilbert = (float*)attribute_buffers[3];
//     for (int i = 0; i < nrows; i++) {
//         EXPECT_EQ(hilbert[i * 2], hilbert[i * 2 + 1]) << "Unexpected value at index " << i;
//         EXPECT_NEAR(hilbert[i * 2], hilbert_expected[i], 2e-14) << "Unexpected value at index " << i;
//     }

//     delete subvolume;
// }

// } // namespace
