#ifndef VDS_SLICE_DATAHANDLE_HPP
#define VDS_SLICE_DATAHANDLE_HPP

#include <OpenVDS/OpenVDS.h>
#include <functional>

#include "metadatahandle.hpp"
#include "subcube.hpp"

using voxel = float[OpenVDS::Dimensionality_Max];

class DataHandle {

public:
    virtual ~DataHandle() {};

    virtual MetadataHandle const& get_metadata() const noexcept(true) = 0;

    virtual std::int64_t samples_buffer_size(std::size_t const nsamples) noexcept(false) = 0;

    virtual void read_samples(
        void* const buffer,
        std::int64_t const size,
        voxel const* samples,
        std::size_t const nsamples,
        enum interpolation_method const interpolation_method
    ) noexcept(false) = 0;

    virtual std::int64_t subcube_buffer_size(SubCube const& subcube) noexcept(false) = 0;

    virtual void read_subcube(
        void* const buffer,
        std::int64_t size,
        SubCube const& subcube
    ) noexcept(false) = 0;

    virtual std::int64_t traces_buffer_size(std::size_t const ntraces) noexcept(false) = 0;

    virtual void read_traces(
        void* const buffer,
        std::int64_t const size,
        voxel const* coordinates,
        std::size_t const ntraces,
        enum interpolation_method const interpolation_method
    ) noexcept(false) = 0;

    static OpenVDS::VolumeDataFormat format() noexcept(true);
};

class SingleDataHandle : public DataHandle {
    SingleDataHandle(OpenVDS::VDSHandle handle);
    friend SingleDataHandle* make_single_datahandle(const char* url, const char* credentials);

public:
    MetadataHandle const& get_metadata() const noexcept(true);

    std::int64_t subcube_buffer_size(SubCube const& subcube) noexcept(false);

    void read_subcube(
        void* const buffer,
        std::int64_t size,
        SubCube const& subcube
    ) noexcept(false);

    std::int64_t traces_buffer_size(std::size_t const ntraces) noexcept(false);

    void read_traces(
        void* const buffer,
        std::int64_t const size,
        voxel const* coordinates,
        std::size_t const ntraces,
        enum interpolation_method const interpolation_method
    ) noexcept(false);

    std::int64_t samples_buffer_size(std::size_t const nsamples) noexcept(false);

    void read_samples(
        void* const buffer,
        std::int64_t const size,
        voxel const* samples,
        std::size_t const nsamples,
        enum interpolation_method const interpolation_method
    ) noexcept(false);

private:
    OpenVDS::ScopedVDSHandle m_file_handle;
    OpenVDS::VolumeDataAccessManager m_access_manager;
    SingleMetadataHandle m_metadata;

    static int constexpr lod_level = 0;
    static int constexpr channel = 0;
};

SingleDataHandle* make_single_datahandle(
    const char* url,
    const char* credentials
) noexcept(false);

class DoubleDataHandle : public DataHandle {

public:
    DoubleDataHandle(OpenVDS::VDSHandle handle_a, OpenVDS::VDSHandle handle_b, enum binary_operator binary_symbol);
    friend DoubleDataHandle* make_double_datahandle(
        const char* url_a,
        const char* credentials_a,
        const char* url_b,
        const char* credentials_b,
        enum binary_operator binary_symbol
    );

    MetadataHandle const& get_metadata() const noexcept(true);

    std::int64_t subcube_buffer_size(SubCube const& subcube) noexcept(false);

    void read_subcube(
        void* const buffer,
        std::int64_t size,
        SubCube const& subcube
    ) noexcept(false);

    std::int64_t traces_buffer_size(std::size_t const ntraces) noexcept(false);

    void read_traces(
        void* const buffer,
        std::int64_t const size,
        voxel const* coordinates,
        std::size_t const ntraces,
        enum interpolation_method const interpolation_method
    ) noexcept(false);

    std::int64_t samples_buffer_size(std::size_t const nsamples) noexcept(false);

    void read_samples(
        void* const buffer,
        std::int64_t const size,
        voxel const* samples,
        std::size_t const nsamples,
        enum interpolation_method const interpolation_method
    ) noexcept(false);

private:
    OpenVDS::ScopedVDSHandle m_file_handle_a;
    OpenVDS::ScopedVDSHandle m_file_handle_b;
    OpenVDS::VolumeDataAccessManager m_access_manager_a;
    OpenVDS::VolumeDataAccessManager m_access_manager_b;
    DoubleMetadataHandle m_metadata;
    SingleMetadataHandle m_metadata_a;
    SingleMetadataHandle m_metadata_b;
    std::function<void(float*, const float*, std::size_t)> m_binary_operator;
    enum binary_operator m_binary_symbol;

    static int constexpr lod_level = 0;
    static int constexpr channel = 0;

    SubCube offset_bounds(const SubCube subcube, SingleMetadataHandle metadata);
    void extract_part_of_trace(
        std::vector<float> coordinates,
        std::vector<float> source_traces,
        int source_trace_length,
        float* target_buffer
    );
};

DoubleDataHandle* make_double_datahandle(
    const char* url_a, const char* credentials_a,
    const char* url_b, const char* credentials_b,
    enum binary_operator bin_operator
) noexcept(false);

void inplace_subtraction(
    float* buffer_A,
    const float* buffer_B,
    std::size_t nsamples
) noexcept(true);

void inplace_addition(
    float* buffer_A,
    const float* buffer_B,
    std::size_t nsamples
) noexcept(true);

void inplace_multiplication(
    float* buffer_A,
    const float* buffer_B,
    std::size_t nsamples
) noexcept(true);

void inplace_division(
    float* buffer_A,
    const float* buffer_B,
    std::size_t nsamples
) noexcept(true);

#endif /* VDS_SLICE_DATAHANDLE_HPP */
