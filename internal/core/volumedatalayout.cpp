#include "volumedatalayout.hpp"
#include <iostream>

DoubleVolumeDataLayout::DoubleVolumeDataLayout(
    OpenVDS::VolumeDataLayout const* const layout_a, 
    OpenVDS::VolumeDataLayout const* const layout_b):
        m_layout_a(layout_a),
        m_layout_b(layout_b) {}

OpenVDS::VolumeDataAxisDescriptor DoubleVolumeDataLayout::GetAxisDescriptor(int dimension){
      return OpenVDS::VolumeDataAxisDescriptor(
        GetDimensionNumSamples(dimension),
        GetDimensionName(dimension),
        GetDimensionUnit(dimension),
        GetDimensionMin(dimension),
        GetDimensionMax(dimension));
}
