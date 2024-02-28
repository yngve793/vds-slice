#ifndef VDS_SLICE_AXIS_HPP
#define VDS_SLICE_AXIS_HPP

#include <memory>
#include <string>

#include <OpenVDS/OpenVDS.h>

class BaseAxis {
public:
    virtual int nsamples() const noexcept(true) = 0;

    virtual float min() const noexcept(true) = 0;
    virtual float max() const noexcept(true) = 0;

    virtual float stepsize() const noexcept(true) = 0;

    virtual std::string unit() const noexcept(true) = 0;
    virtual int dimension() const noexcept(true) = 0;

    virtual std::string name() const noexcept(true) = 0;

    virtual bool inrange(float coordinate) const noexcept(true) = 0;
    virtual float to_sample_position(float coordinate) const noexcept(false) = 0;
    virtual void assert_equal(BaseAxis const& other) noexcept(false) = 0;
};

class SingleAxis : public BaseAxis {

public:
    SingleAxis(OpenVDS::VolumeDataLayout const* const layout, int const dimension);

    int nsamples() const noexcept(true);

    float min() const noexcept(true);
    float max() const noexcept(true);

    float stepsize() const noexcept(true);

    std::string unit() const noexcept(true);
    int dimension() const noexcept(true);

    std::string name() const noexcept(true);

    bool inrange(float coordinate) const noexcept(true);
    float to_sample_position(float coordinate) const noexcept(false);

    void assert_equal(BaseAxis const& other) noexcept(false);

private:
    int const m_dimension;
    OpenVDS::VolumeDataAxisDescriptor m_axis_descriptor;
};

class DoubleAxis : public BaseAxis {

public:
    DoubleAxis(SingleAxis axis_a, SingleAxis axis_b);

    int nsamples() const noexcept(true);

    float min() const noexcept(true);
    float max() const noexcept(true);

    float stepsize() const noexcept(true);

    std::string unit() const noexcept(true);
    int dimension() const noexcept(true);

    std::string name() const noexcept(true);

    bool inrange(float coordinate) const noexcept(true);
    float to_sample_position(float coordinate) const noexcept(false);

    void assert_stepsize() const noexcept(false);

    void assert_overlap() const noexcept(false);

    void assert_equal(BaseAxis const& other) noexcept(false);

    void assert_name() noexcept(false);

private:
    SingleAxis m_axis_a;
    SingleAxis m_axis_b;
    // int const m_dimension;
    OpenVDS::VolumeDataAxisDescriptor m_axis_descriptor;
};

#endif /* VDS_SLICE_AXIS_HPP */
