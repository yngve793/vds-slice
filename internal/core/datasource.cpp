#include "datasource.hpp"
#include "datahandle.hpp"

SingleDataSource::SingleDataSource(const char* url, const char* credentials) {
    this->handle = make_datahandle(url, credentials);
}

SingleDataSource::~SingleDataSource() {
    if (this->handle != NULL) {
        delete (this->handle);
        this->handle = NULL;
    }
}

MetadataHandle const& SingleDataSource::get_metadata() const noexcept(true) {
    return this->handle->get_metadata();
}

std::int64_t SingleDataSource::samples_buffer_size(std::size_t const nsamples) noexcept(false) {
    return this->handle->samples_buffer_size(nsamples);
}

void SingleDataSource::read_samples(
    void* const buffer,
    std::int64_t const size,
    voxel const* samples,
    std::size_t const nsamples,
    enum interpolation_method const interpolation_method
) noexcept(false) {
    return this->handle->read_samples(buffer, size, samples, nsamples, interpolation_method);
}

std::int64_t SingleDataSource::subcube_buffer_size(SubCube const& subcube) noexcept(false) {
    return this->handle->subcube_buffer_size(subcube);
}

void SingleDataSource::read_subcube(
    void* const buffer,
    std::int64_t size,
    SubCube const& subcube
) noexcept(false) {
    this->handle->read_subcube(buffer, size, subcube);
}

void SingleDataSource::read_traces(
    void* const buffer,
    std::int64_t const size,
    voxel const* coordinates,
    std::size_t const ntraces,
    enum interpolation_method const interpolation_method
) noexcept(false) {

    this->handle->read_traces(buffer, size, coordinates, ntraces, interpolation_method);
}

std::int64_t SingleDataSource::traces_buffer_size(std::size_t const ntraces) noexcept(false) {
    return this->handle->traces_buffer_size(ntraces);
}

SingleDataSource* make_single_datasource(
    const char* url,
    const char* credentials
) {
    return new SingleDataSource(url, credentials);
}

DoubleDataSource::DoubleDataSource(
    const char* url_A, const char* credentials_A,
    const char* url_B, const char* credentials_B,
    void (*func)(float*, float*, float*, std::size_t)
) {
    this->handle_A = make_single_datasource(url_A, credentials_A);
    this->handle_B = make_single_datasource(url_B, credentials_B);
    this->func = func;
    this->metadata = new DoubleMetadataHandle(
        this->handle_A->get_metadata(),
        this->handle_B->get_metadata()
    );
}

DoubleDataSource::~DoubleDataSource() {
    if (this->handle_A != NULL) {
        delete (this->handle_A);
        this->handle_A = NULL;
    }
    if (this->handle_B != NULL) {
        delete (this->handle_B);
        this->handle_B = NULL;
    }
}

void DoubleDataSource::validate_metadata() const noexcept(false) {

    // SingleMetadataHandle mdh_A = this->handle_A->get_metadata();
    // SingleMetadataHandle mdh_B = this->handle_B->get_metadata();

    // mdh_A.iline().validate_compatible(mdh_B.iline());
    // mdh_A.xline().validate_compatible(mdh_B.xline());
    // mdh_A.sample().validate_compatible(mdh_B.sample());
}

MetadataHandle const& DoubleDataSource::get_metadata() const noexcept(true) {
    return this->handle_A->get_metadata();
}

std::int64_t DoubleDataSource::samples_buffer_size(std::size_t const nsamples) noexcept(false) {
    return this->handle_A->samples_buffer_size(nsamples);
}

std::int64_t DoubleDataSource::subcube_buffer_size(SubCube const& subcube) noexcept(false) {
    return this->handle_A->subcube_buffer_size(subcube);
}

void DoubleDataSource::read_subcube(
    void* const buffer,
    std::int64_t size,
    SubCube const& subcube
) noexcept(false) {
    std::vector<float> buffer_A((int)size / sizeof(float));
    std::vector<float> buffer_B((int)size / sizeof(float));
    this->handle_A->read_subcube(buffer_A.data(), size, subcube);
    this->handle_B->read_subcube(buffer_B.data(), size, subcube);

    this->func(buffer_A.data(), buffer_B.data(), (float*)buffer, (int)size / sizeof(float));
}

std::int64_t DoubleDataSource::traces_buffer_size(
    std::size_t const ntraces
) noexcept(false) {
    return this->handle_A->traces_buffer_size(ntraces);
}

/// @brief Returns apply the function to trace from source A and B
/// @param buffer to float matrix of trace_length x ntraces
/// @param size Number of bytes in buffer
/// @param coordinates
/// @param ntraces
/// @param interpolation_method
void DoubleDataSource::read_traces(
    void* const buffer,
    std::int64_t const size,
    voxel const* coordinates,
    std::size_t const ntraces,
    interpolation_method const interpolation_method
) noexcept(false) {

    std::vector<float> buffer_A((int)size / sizeof(float));
    std::vector<float> buffer_B((int)size / sizeof(float));

    this->handle_A->read_traces(buffer_A.data(), size, coordinates, ntraces, interpolation_method);
    this->handle_B->read_traces(buffer_B.data(), size, coordinates, ntraces, interpolation_method);

    this->func(buffer_A.data(), buffer_B.data(), (float*)buffer, (int)size / sizeof(float));
}

void DoubleDataSource::read_samples(
    void* const buffer,
    std::int64_t const size,
    voxel const* samples,
    std::size_t const nsamples,
    interpolation_method const interpolation_method
) noexcept(false) {

    std::vector<float> buffer_A(nsamples);
    std::vector<float> buffer_B(nsamples);

    this->handle_A->read_samples(buffer_A.data(), size, samples, nsamples, interpolation_method);
    this->handle_B->read_samples(buffer_B.data(), size, samples, nsamples, interpolation_method);

    this->func(buffer_A.data(), buffer_B.data(), (float*)buffer, nsamples);
}

DoubleDataSource* make_double_datasource(
    const char* url_A,
    const char* credentials_A,
    const char* url_B,
    const char* credentials_B,
    void (*func)(float*, float*, float*, std::size_t)
) noexcept(false) {
    return new DoubleDataSource(url_A, credentials_A, url_B, credentials_B, func);
}

void subtraction(float* buffer_A, float* buffer_B, float* out_buffer, std::size_t nsamples) noexcept(false) {
    for (std::size_t i = 0; i < nsamples; i++) {
        out_buffer[i] = buffer_A[i] - buffer_B[i];
    }
}

void addition(float* buffer_A, float* buffer_B, float* out_buffer, std::size_t nsamples) noexcept(false) {
    for (std::size_t i = 0; i < nsamples; i++) {
        out_buffer[i] = buffer_A[i] + buffer_B[i];
    }
}

void multiplication(float* buffer_A, float* buffer_B, float* out_buffer, std::size_t nsamples) noexcept(false) {
    for (std::size_t i = 0; i < nsamples; i++) {
        out_buffer[i] = buffer_A[i] * buffer_B[i];
    }
}

void division(float* buffer_A, float* buffer_B, float* out_buffer, std::size_t nsamples) noexcept(false) {
    for (std::size_t i = 0; i < nsamples; i++) {
        out_buffer[i] = buffer_A[i] / buffer_B[i];
    }
}
