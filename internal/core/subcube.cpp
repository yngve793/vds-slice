#include "subcube.hpp"

#include <stdexcept>

#include "axis.hpp"
#include "ctypes.h"
#include "exceptions.hpp"
#include "metadatahandle.hpp"
#include "utils.hpp"

namespace {

int lineno_annotation_to_voxel(
    int lineno,
    Axis const& axis
) {
    float min = axis.min();
    float max = axis.max();
    float stepsize = axis.stepsize();

    float voxelline = (lineno - min) / stepsize;

    if (lineno < min || lineno > max || std::floor(voxelline) != voxelline) {
        throw detail::bad_request(
            "Invalid lineno: " + std::to_string(lineno) +
            ", valid range: [" + utils::to_string_with_precision(min) +
            ":" + utils::to_string_with_precision(max) +
            ":" + utils::to_string_with_precision(stepsize) + "]"
        );
    }

    return voxelline;
}

int lineno_index_to_voxel(
    int lineno,
    Axis const& axis
) {
    /* Line-numbers in IJK match Voxel - do bound checking and return*/
    int min = 0;
    int max = axis.nsamples() - 1;

    if (lineno < min || lineno > max) {
        throw detail::bad_request(
            "Invalid lineno: " + std::to_string(lineno) +
            ", valid range: [" + std::to_string(min) +
            ":" + std::to_string(max) +
            ":1]"
        );
    }

    return lineno;
}

int to_voxel(
    Axis const& axis,
    int const lineno,
    enum coordinate_system const system
) {
    switch (system) {
        case ANNOTATION: {
            return ::lineno_annotation_to_voxel(lineno, axis);
        }
        case INDEX: {
            return ::lineno_index_to_voxel(lineno, axis);
        }
        default: {
            throw std::runtime_error("Unhandled coordinate system");
        }
    }
}

} /* namespace */

SubCube::SubCube(MetadataHandle const& metadata) {
    auto const& iline = metadata.iline();
    auto const& xline = metadata.xline();
    auto const& sample = metadata.sample();

    this->bounds.upper[iline.dimension()] = iline.nsamples();
    this->bounds.upper[xline.dimension()] = xline.nsamples();
    this->bounds.upper[sample.dimension()] = sample.nsamples();
}

void SubCube::constrain(
    MetadataHandle const& metadata,
    std::vector<Bound> const& bounds
) noexcept(false) {
    for (auto const& bound : bounds) {
        auto direction = Direction(bound.name);
        auto system = direction.coordinate_system();
        auto axis = metadata.get_axis(direction);

        auto lower = ::to_voxel(axis, bound.lower, system);
        auto upper = ::to_voxel(axis, bound.upper, system);

        this->bounds.lower[axis.dimension()] = lower;
        this->bounds.upper[axis.dimension()] = upper + 1; // inclusive
    }
}

void SubCube::set_slice(
    Axis const& axis,
    int const lineno,
    enum coordinate_system const coordinate_system
) {
    int voxelline = ::to_voxel(axis, lineno, coordinate_system);

    this->bounds.lower[axis.dimension()] = voxelline;
    this->bounds.upper[axis.dimension()] = voxelline + 1;
}
