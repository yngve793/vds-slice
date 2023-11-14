#ifndef VDS_SLICE_DATA_SOURCE_HPP
#define VDS_SLICE_DATA_SOURCE_HPP

#include "datahandle.hpp"
#include "metadatahandle.hpp"

using voxel = float[OpenVDS::Dimensionality_Max];

/// @brief Generic data source
class DataSource {

public:
    virtual ~DataSource() {}

    virtual MetadataHandle const &get_metadata() const noexcept(true) = 0;

    virtual std::int64_t samples_buffer_size(std::size_t const nsamples) noexcept(false) = 0;

    virtual void read_samples(
        void *const buffer,
        std::int64_t const size,
        voxel const *samples,
        std::size_t const nsamples,
        enum interpolation_method const interpolation_method) noexcept(false) = 0;

    virtual std::int64_t subcube_buffer_size(SubCube const &subcube) noexcept(false) = 0;

    virtual void read_subcube(
        void *const buffer,
        std::int64_t size,
        SubCube const &subcube) noexcept(false) = 0;

    virtual std::int64_t traces_buffer_size(std::size_t const ntraces) noexcept(false) = 0;

    virtual void read_traces(
        void *const buffer,
        std::int64_t const size,
        voxel const *coordinates,
        std::size_t const ntraces,
        enum interpolation_method const interpolation_method) noexcept(false) = 0;
};

class SingleDataSource : public DataSource {

public:
    SingleDataSource(const char *url, const char *credentials);

    ~SingleDataSource();

    MetadataHandle const &get_metadata() const noexcept(true);

    std::int64_t samples_buffer_size(std::size_t const nsamples) noexcept(false);

    void read_samples(
        void *const buffer,
        std::int64_t const size,
        voxel const *samples,
        std::size_t const nsamples,
        enum interpolation_method const interpolation_method) noexcept(false);

    std::int64_t subcube_buffer_size(SubCube const &subcube) noexcept(false);

    void read_subcube(
        void *const buffer,
        std::int64_t size,
        SubCube const &subcube) noexcept(false);

    void read_traces(
        void *const buffer,
        std::int64_t const size,
        voxel const *coordinates,
        std::size_t const ntraces,
        enum interpolation_method const interpolation_method) noexcept(false);

    std::int64_t traces_buffer_size(std::size_t const ntraces) noexcept(false);

private:
    DataHandle *handle;
};

SingleDataSource *make_single_datasource(
    const char *url,
    const char *credentials) noexcept(false);

class DoubleDataSource : public DataSource {

public:
    DoubleDataSource(
        const char *url_A,
        const char *credentials_A,
        const char *url_B,
        const char *credentials_B,
        void (*func)(float *, float *, float *, std::size_t));

    ~DoubleDataSource();

    void validate_metadata() const noexcept(false);

    MetadataHandle const &get_metadata() const noexcept(true);

    std::int64_t samples_buffer_size(std::size_t const nsamples) noexcept(false);

    void read_samples(
        void *const buffer,
        std::int64_t const size,
        voxel const *samples,
        std::size_t const nsamples,
        enum interpolation_method const interpolation_method) noexcept(false);

    std::int64_t subcube_buffer_size(SubCube const &subcube) noexcept(false);

    void read_subcube(
        void *const buffer,
        std::int64_t size,
        SubCube const &subcube) noexcept(false);

    void read_traces(
        void *const buffer,
        std::int64_t const size,
        voxel const *coordinates,
        std::size_t const ntraces,
        enum interpolation_method const interpolation_method) noexcept(false);

    std::int64_t traces_buffer_size(std::size_t const ntraces) noexcept(false);

private:
    DataHandle *handle_A;
    DataHandle *handle_B;
    void (*func)(float *, float *, float *, std::size_t);
};

DoubleDataSource *make_double_datasource(
    const char *url_A,
    const char *credentials_A,
    const char *url_B,
    const char *credentials_B,
    void (*func)(float *, float *, float *, std::size_t)) noexcept(false);

void subtraction(
    float *buffer_A,
    float *buffer_B,
    float *out_buffer,
    std::size_t nsamples) noexcept(false);

void addition(
    float *buffer_A,
    float *buffer_B,
    float *out_buffer,
    std::size_t nsamples) noexcept(false);

void multiplication(
    float *buffer_A,
    float *buffer_B,
    float *out_buffer,
    std::size_t nsamples) noexcept(false);

void division(
    float *buffer_A,
    float *buffer_B,
    float *out_buffer,
    std::size_t nsamples) noexcept(false);

#endif /* VDS_SLICE_DATA_SOURCE_HPP */
