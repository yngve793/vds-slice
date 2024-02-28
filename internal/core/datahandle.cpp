#include "datahandle.hpp"

#include <stdexcept>

#include <OpenVDS/KnownMetadata.h>
#include <OpenVDS/OpenVDS.h>

#include "metadatahandle.hpp"
#include "subcube.hpp"

namespace {

OpenVDS::InterpolationMethod to_interpolation(interpolation_method interpolation) {
    switch (interpolation)
    {
        case NEAREST: return OpenVDS::InterpolationMethod::Nearest;
        case LINEAR: return OpenVDS::InterpolationMethod::Linear;
        case CUBIC: return OpenVDS::InterpolationMethod::Cubic;
        case ANGULAR: return OpenVDS::InterpolationMethod::Angular;
        case TRIANGULAR: return OpenVDS::InterpolationMethod::Triangular;
        default: {
            throw std::runtime_error("Unhandled interpolation method");
        }
    }
}

} /* namespace */

SingleDataHandle* make_single_datahandle(
    const char* url,
    const char* credentials
) {
    OpenVDS::Error error;
    auto handle = OpenVDS::Open(url, credentials, error);
    if(error.code != 0) {
        throw std::runtime_error("Could not open VDS: " + error.string);
    }
    return new SingleDataHandle(std::move(handle));
}

SingleDataHandle::SingleDataHandle(OpenVDS::VDSHandle handle)
    : m_file_handle(handle)
    , m_access_manager(OpenVDS::GetAccessManager(handle))
    , m_metadata(m_access_manager.GetVolumeDataLayout())
{}

MetadataHandle const& SingleDataHandle::get_metadata() const noexcept (true) {
    return this->m_metadata;
}

OpenVDS::VolumeDataFormat SingleDataHandle::format() noexcept (true) {
    /*
     * We always want to request data in OpenVDS::VolumeDataFormat::Format_R32
     * format for slice. For fence documentation says: "The traces/samples are
     * always in 32-bit floating point format."
     */
    return OpenVDS::VolumeDataFormat::Format_R32;
}

std::int64_t SingleDataHandle::subcube_buffer_size(
    SubCube const& subcube
) noexcept (false) {
    std::int64_t size = this->m_access_manager.GetVolumeSubsetBufferSize(
        subcube.bounds.lower,
        subcube.bounds.upper,
        SingleDataHandle::format(),
        SingleDataHandle::lod_level,
        SingleDataHandle::channel
    );

    return size;
}

void SingleDataHandle::read_subcube(
    void * const buffer,
    std::int64_t size,
    SubCube const& subcube
) noexcept (false) {
    auto request = this->m_access_manager.RequestVolumeSubset(
        buffer,
        size,
        OpenVDS::Dimensions_012,
        SingleDataHandle::lod_level,
        SingleDataHandle::channel,
        subcube.bounds.lower,
        subcube.bounds.upper,
        SingleDataHandle::format()
    );
    bool const success = request.get()->WaitForCompletion();

    if (!success) {
        throw std::runtime_error("Failed to read from VDS.");
    }
}

std::int64_t SingleDataHandle::traces_buffer_size(std::size_t const ntraces) noexcept (false) {
    int const dimension = this->get_metadata().sample().dimension();
    return this->m_access_manager.GetVolumeTracesBufferSize(ntraces, dimension);
}

void SingleDataHandle::read_traces(
    void * const                    buffer,
    std::int64_t const              size,
    voxel const*                    coordinates,
    std::size_t const               ntraces,
    enum interpolation_method const interpolation_method
) noexcept (false) {
    int const dimension = this->get_metadata().sample().dimension();

    auto request = this->m_access_manager.RequestVolumeTraces(
        (float*)buffer,
        size,
        OpenVDS::Dimensions_012,
        SingleDataHandle::lod_level,
        SingleDataHandle::channel,
        coordinates,
        ntraces,
        ::to_interpolation(interpolation_method),
        dimension
    );
    bool const success = request.get()->WaitForCompletion();

    if (!success) {
        throw std::runtime_error("Failed to read from VDS.");
    }
}

std::int64_t SingleDataHandle::samples_buffer_size(
    std::size_t const nsamples
) noexcept (false) {
    return this->m_access_manager.GetVolumeSamplesBufferSize(
        nsamples,
        SingleDataHandle::channel
    );
}

void SingleDataHandle::read_samples(
    void * const                    buffer,
    std::int64_t const              size,
    voxel const*                    samples,
    std::size_t const               nsamples,
    enum interpolation_method const interpolation_method
) noexcept (false) {
    auto request = this->m_access_manager.RequestVolumeSamples(
        (float*)buffer,
        size,
        OpenVDS::Dimensions_012,
        SingleDataHandle::lod_level,
        SingleDataHandle::channel,
        samples,
        nsamples,
        ::to_interpolation(interpolation_method)
    );

    bool const success = request.get()->WaitForCompletion();
    if (!success) {
        throw std::runtime_error("Failed to read from VDS.");
    }
}

DoubleDataHandle* make_double_datahandle(
    const char* url_a,
    const char* credentials_a,
    const char* url_b,
    const char* credentials_b,
    binary_function binary_operator
) noexcept(false) {
    OpenVDS::Error error;
    auto handle_a = OpenVDS::Open(url_a, credentials_a, error);
    auto handle_b = OpenVDS::Open(url_b, credentials_b, error);
    if (error.code != 0) {
        throw std::runtime_error("Could not open VDS: " + error.string);
    }
    return new DoubleDataHandle(std::move(handle_a), std::move(handle_b), binary_operator);
}

DoubleDataHandle::DoubleDataHandle(OpenVDS::VDSHandle handle_a, OpenVDS::VDSHandle handle_b, binary_function binary_operator)
    : m_file_handle_a(handle_a)
    , m_file_handle_b(handle_b)
    , m_binary_operator(binary_operator)
    , m_access_manager_a(OpenVDS::GetAccessManager(handle_a))
    , m_access_manager_b(OpenVDS::GetAccessManager(handle_b))
    , m_metadata_a(m_access_manager_a.GetVolumeDataLayout())
    , m_metadata_b(m_access_manager_b.GetVolumeDataLayout())
    , m_layout(DoubleVolumeDataLayout(m_access_manager_a.GetVolumeDataLayout(), m_access_manager_b.GetVolumeDataLayout()))
    , m_metadata(DoubleMetadataHandle(&m_layout))
    {}

MetadataHandle const& DoubleDataHandle::get_metadata() const noexcept(true) {
    return this->m_metadata;
}

OpenVDS::VolumeDataFormat DoubleDataHandle::format() noexcept(true) {
    /*
     * We always want to request data in OpenVDS::VolumeDataFormat::Format_R32
     * format for slice. For fence documentation says: "The traces/samples are
     * always in 32-bit floating point format."
     */
    return OpenVDS::VolumeDataFormat::Format_R32;
}

std::int64_t DoubleDataHandle::subcube_buffer_size(
    SubCube const& subcube
) noexcept(false) {
    std::int64_t size = this->m_access_manager_a.GetVolumeSubsetBufferSize(
        subcube.bounds.lower,
        subcube.bounds.upper,
        DoubleDataHandle::format(),
        DoubleDataHandle::lod_level,
        DoubleDataHandle::channel
    );

    return size;
}

SubCube DoubleDataHandle::offset_bounds(SubCube subcube, SingleMetadataHandle m_metadata_new) {

    SubCube new_subcube = std::move(subcube);

    float iline_offset = (m_metadata.iline().min() - m_metadata_new.iline().min()) / m_metadata.iline().stepsize();
    float xline_offset = (m_metadata.xline().min() - m_metadata_new.xline().min()) / m_metadata.xline().stepsize();
    float sample_offset = (m_metadata.sample().min() - m_metadata_new.sample().min()) / m_metadata.sample().stepsize();

    new_subcube.bounds.lower[m_metadata.iline().dimension()] += iline_offset;
    new_subcube.bounds.lower[m_metadata.xline().dimension()] += xline_offset;
    new_subcube.bounds.lower[m_metadata.sample().dimension()] += sample_offset;

    new_subcube.bounds.upper[m_metadata.iline().dimension()] += iline_offset;
    new_subcube.bounds.upper[m_metadata.xline().dimension()] += xline_offset;
    new_subcube.bounds.upper[m_metadata.sample().dimension()] += sample_offset;
    return new_subcube;
}

void DoubleDataHandle::read_subcube(
    void* const buffer,
    std::int64_t size,
    SubCube const& subcube
) noexcept(false) {

    SubCube subcube_a = this->offset_bounds(subcube, m_metadata_a);
    std::vector<char> buffer_a(size);

    auto request_a = this->m_access_manager_a.RequestVolumeSubset(
        buffer, // buffer_a.data(),
        size,
        OpenVDS::Dimensions_012,
        DoubleDataHandle::lod_level,
        DoubleDataHandle::channel,
        subcube_a.bounds.lower,
        subcube_a.bounds.upper,
        DoubleDataHandle::format()
    );

    SubCube subcube_b = this->offset_bounds(subcube, m_metadata_b);
    std::vector<char> buffer_b(size);
    auto request_b = this->m_access_manager_b.RequestVolumeSubset(
        buffer_b.data(),
        size,
        OpenVDS::Dimensions_012,
        DoubleDataHandle::lod_level,
        DoubleDataHandle::channel,
        subcube_b.bounds.lower,
        subcube_b.bounds.upper,
        DoubleDataHandle::format()
    );

    bool const success_a = request_a.get()->WaitForCompletion();
    bool const success_b = request_b.get()->WaitForCompletion();

    if (!success_a && !success_b) {
        throw std::runtime_error("Failed to read from VDS.");
    }

    // m_binary_operator((float* const)buffer_a.data(), (float* const)buffer_b.data(), (std::size_t)size/sizeof(float));
    m_binary_operator((float*)buffer, (float* const)buffer_b.data(), (std::size_t)size / sizeof(float));
}

std::int64_t DoubleDataHandle::traces_buffer_size(std::size_t const ntraces) noexcept(false) {
    int const dimension = this->get_metadata().sample().dimension();
    return this->m_access_manager_a.GetVolumeTracesBufferSize(ntraces, dimension);
}

void DoubleDataHandle::read_traces(
    void* const buffer,
    std::int64_t const size,
    voxel const* coordinates,
    std::size_t const ntraces,
    enum interpolation_method const interpolation_method
) noexcept(false) {
    int const dimension = this->get_metadata().sample().dimension();

    auto request = this->m_access_manager_a.RequestVolumeTraces(
        (float*)buffer,
        size,
        OpenVDS::Dimensions_012,
        DoubleDataHandle::lod_level,
        DoubleDataHandle::channel,
        coordinates,
        ntraces,
        ::to_interpolation(interpolation_method),
        dimension
    );
    bool const success = request.get()->WaitForCompletion();

    if (!success) {
        throw std::runtime_error("Failed to read from VDS.");
    }
}

std::int64_t DoubleDataHandle::samples_buffer_size(
    std::size_t const nsamples
) noexcept(false) {
    return this->m_access_manager_a.GetVolumeSamplesBufferSize(
        nsamples,
        DoubleDataHandle::channel
    );
}

void DoubleDataHandle::read_samples(
    void* const buffer,
    std::int64_t const size,
    voxel const* samples,
    std::size_t const nsamples,
    enum interpolation_method const interpolation_method
) noexcept(false) {
    auto request = this->m_access_manager_a.RequestVolumeSamples(
        (float*)buffer,
        size,
        OpenVDS::Dimensions_012,
        DoubleDataHandle::lod_level,
        DoubleDataHandle::channel,
        samples,
        nsamples,
        ::to_interpolation(interpolation_method)
    );

    bool const success = request.get()->WaitForCompletion();
    if (!success) {
        throw std::runtime_error("Failed to read from VDS.");
    }
}
