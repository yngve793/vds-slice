#include "volumedatalayout.hpp"
#include <OpenVDS/VolumeDataAxisDescriptor.h>
#include <iostream>
// #include <stdexcept>
#include "exceptions.hpp"

DoubleVolumeDataLayout::DoubleVolumeDataLayout(
    OpenVDS::VolumeDataLayout const* const layout_a,
    OpenVDS::VolumeDataLayout const* const layout_b
) : m_layout_a(layout_a),
    m_layout_b(layout_b) {

    OpenVDS::VDSIJKGridDefinition vds_a = m_layout_a->GetVDSIJKGridDefinitionFromMetadata();
    OpenVDS::VDSIJKGridDefinition vds_b = m_layout_b->GetVDSIJKGridDefinitionFromMetadata();

    if (vds_a.dimensionMap != vds_b.dimensionMap)
        throw std::runtime_error("Mismatch in VDS dimension map");

    if (vds_a.iUnitStep != vds_b.iUnitStep)
        throw std::runtime_error("Mismatch in iUnitStep");

    if (vds_a.jUnitStep != vds_b.jUnitStep)
        throw std::runtime_error("Mismatch in jUnitStep");

    if (vds_a.kUnitStep != vds_b.kUnitStep)
        throw std::runtime_error("Mismatch in kUnitStep");

    // origin consist of three values ()
    if (vds_a.origin[0] == vds_b.origin[0] && vds_a.origin[1] == vds_b.origin[1])

        // if (!(vds_a == vds_b)) {
        //     throw std::runtime_error("The two provided data sources do not have a identical origin, increment vectors and dimension ordering");
        // }

        if (m_layout_a->GetDimensionality() != m_layout_b->GetDimensionality()) {
            throw detail::bad_request("Different number of dimensions");
        }

    for (int32_t dimension = 0; dimension < Dimensionality_Max; dimension++) {

        if (dimension < this->GetDimensionality()) {

            std::cout << dimension << " a " << m_layout_a->GetDimensionMin(dimension) << " " << m_layout_a->GetDimensionMax(dimension) << " " << m_layout_a->GetDimensionNumSamples(dimension) << std::endl;
            std::cout << dimension << " b " << m_layout_b->GetDimensionMin(dimension) << " " << m_layout_b->GetDimensionMax(dimension) << " " << m_layout_b->GetDimensionNumSamples(dimension) << std::endl;

            int32_t step_size_a = (m_layout_a->GetDimensionMax(dimension) - m_layout_a->GetDimensionMin(dimension)) / (m_layout_a->GetDimensionNumSamples(dimension) - 1);
            int32_t step_size_b = (m_layout_b->GetDimensionMax(dimension) - m_layout_b->GetDimensionMin(dimension)) / (m_layout_b->GetDimensionNumSamples(dimension) - 1);

            if (step_size_a != step_size_b) {
                throw detail::bad_request("Step size mismatch in axis: " + std::to_string(dimension));
            }

            float offset = (m_layout_b->GetDimensionMin(dimension) - m_layout_a->GetDimensionMin(dimension)) / step_size_a;

            if (std::abs(std::round(offset) - offset) > 0.00001) {
                throw detail::bad_request("Offset mismatch in axis: " + std::to_string(dimension));
            }

            m_dimensionCoordinateMin[dimension] = std::max(
                m_layout_a->GetDimensionMin(dimension),
                m_layout_b->GetDimensionMin(dimension)
            );

            m_dimensionCoordinateMax[dimension] = std::min(
                m_layout_a->GetDimensionMax(dimension),
                m_layout_b->GetDimensionMax(dimension)
            );

            m_dimensionNumSamples[dimension] = 1 + ((m_dimensionCoordinateMax[dimension] - m_dimensionCoordinateMin[dimension]) / step_size_a);

        } else {
            m_dimensionNumSamples[dimension] = 1;
        }
    }
}

int DoubleVolumeDataLayout::GetDimensionality() const {
    // Constructor ensures that dimensionality is the same for m_layout_a and m_layout_b.
    return m_layout_a->GetDimensionality();
}

OpenVDS::VolumeDataAxisDescriptor DoubleVolumeDataLayout::GetAxisDescriptor(int dimension) const{

    // OpenVDS::VolumeDataAxisDescriptor* t = &(OpenVDS::VolumeDataAxisDescriptor());

    return OpenVDS::VolumeDataAxisDescriptor(
        GetDimensionNumSamples(dimension),
        GetDimensionName(dimension),
        GetDimensionUnit(dimension),
        GetDimensionMin(dimension),
        GetDimensionMax(dimension)
    );
}

int DoubleVolumeDataLayout::GetDimensionNumSamples(int dimension) const {

    if (dimension < 0 || dimension >= Dimensionality_Max) {
        int dimMax = Dimensionality_Max;
        throw std::runtime_error("Not valid dimension");
    }
    return m_dimensionNumSamples[dimension];
}

OpenVDS::VDSIJKGridDefinition DoubleVolumeDataLayout::GetVDSIJKGridDefinitionFromMetadata() const {

    // OpenVDS::VDSIJKGridDefinition contains:
    // - 3D origin position for the grid
    // - 3D array for increment vector in i,j,k direction
    // - Dimention map, i.e. order of axis
    return m_layout_a->GetVDSIJKGridDefinitionFromMetadata();
}

const char* DoubleVolumeDataLayout::GetDimensionName(int dimension) const {
    const char* name_a = m_layout_a->GetDimensionName(dimension);
    const char* name_b = m_layout_b->GetDimensionName(dimension);
    if (strcmp(name_a, name_b) == 0){
        return m_layout_a->GetDimensionName(dimension);
    }
    else{
        throw detail::bad_request("Dimension name mismatch for dimension: " + std::to_string(dimension));
    }
}

const char* DoubleVolumeDataLayout::GetDimensionUnit(int dimension) const {
    const char* unit_a = m_layout_a->GetDimensionUnit(dimension);
    const char* unit_b = m_layout_b->GetDimensionUnit(dimension);
    if (strcmp(unit_a, unit_b) == 0){
        return m_layout_a->GetDimensionUnit(dimension);
    }
    else{
        throw detail::bad_request("Dimension unit mismatch for dimension: " + std::to_string(dimension));
    }
}

float DoubleVolumeDataLayout::GetDimensionMin(int dimension) const {
    float min_a = m_layout_a->GetDimensionMin(dimension);
    float min_b = m_layout_b->GetDimensionMin(dimension);
    std::cout << "VolumeDataLayoutMin " << min_a << "  " << min_b << "  " << std::max(min_a, min_b) << std::endl;
    return std::max(min_a, min_b);
}

float DoubleVolumeDataLayout::GetDimensionMax(int dimension) const {
    float min_a = m_layout_a->GetDimensionMax(dimension);
    float min_b = m_layout_b->GetDimensionMax(dimension);
    return std::min(min_a, min_b);
}
