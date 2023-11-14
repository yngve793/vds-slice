#ifndef VDS_SLICE_METADATAHANDLE_HPP
#define VDS_SLICE_METADATAHANDLE_HPP

#include <string>

#include <OpenVDS/OpenVDS.h>

#include "axis.hpp"
#include "boundingbox.hpp"
#include "direction.hpp"

class MetadataHandle{
public:
    // virtual MetadataHandle(OpenVDS::VolumeDataLayout const * const layout)=0;

    virtual Axis iline()  const noexcept (true)=0;
    virtual Axis xline()  const noexcept (true)=0;
    virtual Axis sample() const noexcept (true)= 0;

    virtual BoundingBox bounding_box()      const noexcept (true)= 0;
    virtual std::string crs()               const noexcept (true)= 0;
    virtual std::string input_filename()    const noexcept (true)= 0;
    virtual std::string import_time_stamp() const noexcept (true)= 0;

    virtual Axis get_axis(Direction const direction) const noexcept (false)= 0;

    virtual OpenVDS::IJKCoordinateTransformer coordinate_transformer() const noexcept (true)= 0;

};

class SingleMetadataHandle : public MetadataHandle{
public:
    SingleMetadataHandle(OpenVDS::VolumeDataLayout const * const layout);

    Axis iline()  const noexcept (true);
    Axis xline()  const noexcept (true);
    Axis sample() const noexcept (true);

    BoundingBox bounding_box()      const noexcept (true);
    std::string crs()               const noexcept (true);
    std::string input_filename()    const noexcept (true);
    std::string import_time_stamp() const noexcept (true);

    Axis get_axis(Direction const direction) const noexcept (false);

    OpenVDS::IJKCoordinateTransformer coordinate_transformer() const noexcept (true);
private:
    OpenVDS::VolumeDataLayout const * const m_layout;

    Axis m_iline;
    Axis m_xline;
    Axis m_sample;

    void dimension_validation() const;
    int get_dimension(std::vector<std::string> const & names) const;
};

class DoubleMetadataHandle : public MetadataHandle {
public:
    DoubleMetadataHandle(
        MetadataHandle const& handle_A,
        MetadataHandle const& handle_B);

    Axis iline()  const noexcept (true);
    Axis xline()  const noexcept (true);
    Axis sample() const noexcept (true);

    BoundingBox bounding_box()      const noexcept (true);
    std::string crs()               const noexcept (true);
    std::string input_filename()    const noexcept (true);
    std::string import_time_stamp() const noexcept (true);

    Axis get_axis(Direction const direction) const noexcept (false);

    OpenVDS::IJKCoordinateTransformer coordinate_transformer() const noexcept (true);

private:
    MetadataHandle const * m_handle_A;
    MetadataHandle const * m_handle_B;

    // Axis m_iline;
    // Axis m_xline;
    // Axis m_sample;

};
#endif /* VDS_SLICE_METADATAHANDLE_HPP */
