#ifndef VDS_SLICE_METADATAHANDLE_HPP
#define VDS_SLICE_METADATAHANDLE_HPP

#include <string>

#include <OpenVDS/OpenVDS.h>

#include "axis.hpp"
#include "boundingbox.hpp"
#include "direction.hpp"
#include "volumedatalayout.hpp"
#include "volumedatalayout.hpp"

class MetadataHandle {
    friend class DoubleMetadataHandle;

public:
    virtual BaseAxis& iline() const noexcept(true) = 0;
    virtual BaseAxis& xline() const noexcept(true) = 0;
    virtual BaseAxis& sample() const noexcept(true) = 0;
    virtual BaseAxis& get_axis(Direction const direction) const noexcept(false) = 0;

    virtual BoundingBox bounding_box() const noexcept(false) = 0;
    virtual std::string crs() const noexcept(false) = 0;
    virtual std::string input_filename() const noexcept(false) = 0;
    virtual std::string import_time_stamp() const noexcept(false) = 0;
    virtual OpenVDS::VolumeDataLayout const* const get_layout() const noexcept(false) = 0;
    virtual OpenVDS::IJKCoordinateTransformer coordinate_transformer() const noexcept(false) = 0;

protected:
    virtual void dimension_validation() const = 0;
};

class SingleMetadataHandle : public MetadataHandle {
public:
    SingleMetadataHandle(OpenVDS::VolumeDataLayout const* const layout);

    BaseAxis& iline() const noexcept(true);
    BaseAxis& xline() const noexcept(true);
    BaseAxis& sample() const noexcept(true);
    BaseAxis& get_axis(Direction const direction) const noexcept(false);

    BoundingBox bounding_box() const noexcept(false);
    std::string crs() const noexcept(false);
    std::string input_filename() const noexcept(false);
    std::string import_time_stamp() const noexcept(false);
    OpenVDS::VolumeDataLayout const* const get_layout() const noexcept(false) {
        return this->m_layout;
    }

    OpenVDS::IJKCoordinateTransformer coordinate_transformer() const noexcept(false);

protected:
    void dimension_validation() const;

private:
    OpenVDS::VolumeDataLayout const* const m_layout;

    SingleAxis m_iline;
    SingleAxis m_xline;
    SingleAxis m_sample;

    int get_dimension(std::vector<std::string> const& names) const;
};

class DoubleMetadataHandle : public MetadataHandle {
public:
    DoubleMetadataHandle(DoubleVolumeDataLayout const* const layout);

    BaseAxis& iline() const noexcept(true);
    BaseAxis& xline() const noexcept(true);
    BaseAxis& sample() const noexcept(true);
    BaseAxis& get_axis(Direction const direction) const noexcept(false);

    BoundingBox bounding_box() const noexcept(false);
    std::string crs() const noexcept(false);
    std::string input_filename() const noexcept(false);
    std::string import_time_stamp() const noexcept(false);

    OpenVDS::VolumeDataLayout const* const get_layout() const noexcept(false);

    OpenVDS::IJKCoordinateTransformer coordinate_transformer() const noexcept(false);

    int get_dimension(std::vector<std::string> const& names) const;

protected:
    void dimension_validation() const;

private:
    DoubleVolumeDataLayout const* const m_layout;

    SingleAxis m_iline;
    SingleAxis m_xline;
    SingleAxis m_sample;

    void validate_metadata() const noexcept(false);
    int get_dimension(std::vector<std::string> const& names) const;
};
#endif /* VDS_SLICE_METADATAHANDLE_HPP */
