#ifndef VDS_SLICE_REGULAR_SURFACE_HPP
#define VDS_SLICE_REGULAR_SURFACE_HPP

#include <array>

struct Point {
    double x;
    double y;
};

struct AffineTransformation : private std::array<std::array<double, 3>, 2> {
    using base_type = std::array<std::array<double, 3>, 2>;

    explicit AffineTransformation(base_type x) : base_type(std::move(x)) {}

    Point operator*(Point p) const noexcept(true);

    friend bool operator==(
        AffineTransformation const& left,
        AffineTransformation const& right
    ) noexcept(true);

    static AffineTransformation from_rotation(
        double xori,
        double yori,
        double xinc,
        double yinc,
        double rot
    ) noexcept(true);

    /* Make inverse transformation to the one created from rotation */
    static AffineTransformation inverse_from_rotation(
        double xori,
        double yori,
        double xinc,
        double yinc,
        double rot
    ) noexcept(true);
};

/**
 * Represents a geometrical plane which is seen and intended as a grid.
 *
 * Note that usually the object would resemble a cartesian plane, but as system
 * properties are not checked on creation, it is possible to create a grid where
 * axes are not perpendicular. Anyway mathematical perpendicularity is difficult
 * to achieve here due to floating point errors and plane data coming from other
 * systems. So it is up to the user to assure grid's properties are as desired.
 */
struct Grid {
    Grid(
        double xori,
        double yori,
        double xinc,
        double yinc,
        double rot
    ) : m_transformation(AffineTransformation::from_rotation(xori, yori, xinc, yinc, rot)),
        m_inverse_transformation(
            AffineTransformation::inverse_from_rotation(xori, yori, xinc, yinc, rot)
        ) {}

    /**
     * Compares grids for equality using equality of their affine
     * transformations.
     *
     * Note that in theory grids which differ only by 360 degree rotation
     * should represent the same grid and be considered equal. However due to
     * double precision calculations would differ a bit. Thus note that values
     * here would be equal only when parameters provided in the constructor were
     * equal.
     */
    bool operator==(const Grid& other) const noexcept(true);

    AffineTransformation m_transformation;
    AffineTransformation m_inverse_transformation;
};

struct BoundedGrid : public Grid {
    BoundedGrid(
        Grid grid,
        std::size_t nrows,
        std::size_t ncols
    ) : Grid(grid), m_nrows(nrows), m_ncols(ncols) {}

    /* Grid position (row, col) -> world coordinates */
    Point to_cdp(
        std::size_t const row,
        std::size_t const col
    ) const noexcept(false);

    Point to_cdp(
        std::size_t i
    ) const noexcept(false);

    /* World coordinates -> grid position */
    Point from_cdp(
        Point point
    ) const noexcept(false);

    bool operator==(const BoundedGrid& other) const noexcept(true);

    std::size_t nrows() const noexcept(true) { return this->m_nrows; };
    std::size_t ncols() const noexcept(true) { return this->m_ncols; };
    std::size_t size() const noexcept(true) { return this->ncols() * this->nrows(); };

    std::size_t row(std::size_t i) const noexcept(false);
    std::size_t col(std::size_t i) const noexcept(false);

private:
    std::size_t m_nrows;
    std::size_t m_ncols;
};

std::pair<std::size_t, std::size_t> as_pair(std::size_t row, std::size_t col);

/** Regular Surface - a set of data points over the finite part of 2D grid.
 * It is represented as 2D array with geospacial information. Each array value
 * can mean anything, but in practice it would likely be the depth at the grid
 * position used to calculate the horizon.
 *
 * A regular surface is defined by a 2D regular grid with a shape of nrows *
 * ncols. The grid is located in physical space. The mapping from grid
 * positions (row, col) to a world coordinates is done through an affine
 * transformation [1].
 *
 * The grid itself, although 2D by nature, is represented by a flat C array, in
 * order to pass it between Go and C++ (through C) without copying it.
 *
 * [1] https://en.wikipedia.org/wiki/Affine_transformation
 */
class RegularSurface {

public:
    RegularSurface(
        float* data,
        BoundedGrid grid,
        float fillvalue
    ) : m_data(data),
        m_fillvalue(fillvalue),
        m_grid(grid) {}

    RegularSurface(
        float* data,
        std::size_t nrows,
        std::size_t ncols,
        Grid grid,
        float fillvalue
    ) : RegularSurface(data, BoundedGrid(grid, nrows, ncols), fillvalue) {}

    float(&operator[](std::size_t i) noexcept(false));
    const float(&operator[](std::size_t i) const noexcept(false));

    float(&operator[](std::pair<std::size_t, std::size_t>) noexcept(false));
    const float(&operator[](std::pair<std::size_t, std::size_t>) const noexcept(false));

    float fillvalue() const noexcept(true) { return this->m_fillvalue; };

    std::size_t size() const noexcept(true) { return this->m_grid.size(); };

    BoundedGrid const& grid() const noexcept(true) { return this->m_grid; };

private:
    float* m_data;
    float m_fillvalue;
    const BoundedGrid m_grid;
};

// } // namespace surface

#endif /* VDS_SLICE_REGULAR_SURFACE_HPP */
