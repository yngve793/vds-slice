#include "axis.hpp"

#include "exceptions.hpp"
#include <stdexcept>

#include "utils.hpp"
#include <OpenVDS/IJKCoordinateTransformer.h>
#include <OpenVDS/KnownMetadata.h>

SingleAxis::SingleAxis(
    OpenVDS::VolumeDataLayout const* const layout,
    int const dimension
) : m_dimension(dimension),
    m_axis_descriptor(layout->GetAxisDescriptor(dimension)) {}

float SingleAxis::min() const noexcept(true) {
    return this->m_axis_descriptor.GetCoordinateMin();
}

float SingleAxis::max() const noexcept(true) {
    return this->m_axis_descriptor.GetCoordinateMax();
}

int SingleAxis::nsamples() const noexcept(true) {
    return this->m_axis_descriptor.GetNumSamples();
}

std::string SingleAxis::unit() const noexcept(true) {
    return this->m_axis_descriptor.GetUnit();
}

int SingleAxis::dimension() const noexcept(true) {
    return this->m_dimension;
}

float SingleAxis::stepsize() const noexcept(true) {
    return (this->max() - this->min()) / (this->nsamples() - 1);
}

std::string SingleAxis::name() const noexcept(true) {
    return this->m_axis_descriptor.GetName();
}

bool SingleAxis::inrange(float coordinate) const noexcept(true) {
    return (this->min() - 0.5 * this->stepsize()) <= coordinate &&
           (this->max() + 0.5 * this->stepsize()) > coordinate;
}

float SingleAxis::to_sample_position(float coordinate) const noexcept(false) {
    return this->m_axis_descriptor.CoordinateToSamplePosition(coordinate);
}

void SingleAxis::assert_equal(BaseAxis const& other) noexcept(false) {

    if (this->nsamples() != other.nsamples()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in number of samples: " +
            utils::to_string_with_precision(this->nsamples()) +
            " != " + utils::to_string_with_precision(other.nsamples())
        );
    }

    if (this->min() != other.min()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in min value: " +
            utils::to_string_with_precision(this->min()) +
            " != " + utils::to_string_with_precision(other.min())
        );
    }

    if (this->max() != other.max()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in max value: " +
            utils::to_string_with_precision(this->max()) +
            " != " + utils::to_string_with_precision(other.max())
        );
    }

    // Stepsize is a data integrity check.
    // If min,max and nsamples are equal stepsize is equal for consistent data.
    if (this->stepsize() != other.stepsize()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in stepsize: " +
            std::to_string(this->stepsize()) +
            " != " + std::to_string(other.stepsize())
        );
    }

    if (this->unit() != other.unit()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in unit: " +
            this->unit() +
            " != " + other.unit()
        );
    }

    // Ignore order of dimensions

    if (this->name() != other.name()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in name: " +
            this->name() +
            " != " + other.name()
        );
    }
}

DoubleAxis::DoubleAxis(SingleAxis axis_a, SingleAxis axis_b) : m_axis_a(axis_a), m_axis_b(axis_b) {
    this->assert_name();
    this->assert_overlap();
    this->assert_stepsize();
}

float DoubleAxis::min() const noexcept(true) {
    return std::max(this->m_axis_a.min(), this->m_axis_b.min());
}

float DoubleAxis::max() const noexcept(true) {
    return std::min(this->m_axis_a.max(), this->m_axis_b.max());
}

int DoubleAxis::nsamples() const noexcept(true) {
    return ((this->max() - this->min()) / this->stepsize()) + 1;
}

std::string DoubleAxis::unit() const noexcept(true) {
    // Constructor checks that these are the same.
    return this->m_axis_a.unit();
}

int DoubleAxis::dimension() const noexcept(true) {
    // Constructor checks that dimension is the same. It is not clear if this is required.
    return this->m_axis_a.dimension();
}

float DoubleAxis::stepsize() const noexcept(true) {
    // Constructor checks that these are the same.
    return this->m_axis_a.stepsize();
}

std::string DoubleAxis::name() const noexcept(true) {
    // Constructor checks that these are the same.
    return this->m_axis_a.name();
}

bool DoubleAxis::inrange(float coordinate) const noexcept(true) {
    // Same function as above
    // Is this a bug. min- 0.5 * stepsize is outside??
    return (this->min() - 0.5 * this->stepsize()) <= coordinate &&
           (this->max() + 0.5 * this->stepsize()) > coordinate;
}

float DoubleAxis::to_sample_position(float coordinate) const noexcept(false) {
    // We still need to check that the value is in range
    if (this->m_axis_a.min() > this->m_axis_b.min()) {
        return this->m_axis_a.to_sample_position(coordinate);
    } else {
        return this->m_axis_b.to_sample_position(coordinate);
    }
}

void DoubleAxis::assert_stepsize() const noexcept(false) {

    if (this->m_axis_a.stepsize() != this->m_axis_b.stepsize()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in stepsize: " +
            std::to_string(this->m_axis_a.stepsize()) +
            " != " + std::to_string(this->m_axis_b.stepsize())
        );
    }
}

void DoubleAxis::assert_overlap() const noexcept(false) {

    if (this->min() > this->max()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": No data overlap: min = " +
            std::to_string(this->min()) +
            " and max = " + std::to_string(this->max())
        );
    }
}

void DoubleAxis::assert_name() noexcept(false) {

    if (this->m_axis_a.name() != this->m_axis_b.name()) {
        throw detail::bad_request(
            "Axis: " + this->name() +
            ": Mismatch in name: " +
            this->m_axis_a.name() +
            " != " + this->m_axis_b.name()
        );
    }
}

void DoubleAxis::assert_equal(BaseAxis const& other) noexcept(false) {
    throw std::runtime_error("Not implemented: DoubleVolumeDataLayout::GetLayoutHash");
}
