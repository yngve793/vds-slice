#include <cmath>
#include <stdexcept>

#include "regularsurface.hpp"

Point AffineTransformation::operator*(Point p) const noexcept(true) {
    return {
        this->at(0)[0] * p.x + this->at(0)[1] * p.y + this->at(0)[2],
        this->at(1)[0] * p.x + this->at(1)[1] * p.y + this->at(1)[2],
    };
};

bool operator==(
    AffineTransformation const& left,
    AffineTransformation const& right
) noexcept(true) {
    const auto& lhs = static_cast<const AffineTransformation::base_type&>(left);
    const auto& rhs = static_cast<const AffineTransformation::base_type&>(right);

    return lhs == rhs;
};

AffineTransformation AffineTransformation::from_rotation(
    double xori,
    double yori,
    double xinc,
    double yinc,
    double rot
) noexcept(true) {
    double rad = rot * (M_PI / 180);
    /**
     * Matrix is composed by applying affine transformations [1] in the
     * following order:
     * - scaling by xinc, yinc
     * - counterclockwise rotation by angle rad around the center
     * - translation by the offset (xori, yori)
     *
     * By scaling unit vectors, rotating coordinate system axes and moving
     * coordinate system center to new position we transform index-based
     * rows-and-columns cartesian coordinate system into CDP-surface one.
     *
     * [1] https://en.wikipedia.org/wiki/Affine_transformation
     */
    return AffineTransformation(base_type({{xinc * std::cos(rad), -yinc * std::sin(rad), xori, xinc * std::sin(rad), yinc * std::cos(rad), yori}}));
}

AffineTransformation AffineTransformation::inverse_from_rotation(
    double xori,
    double yori,
    double xinc,
    double yinc,
    double rot
) noexcept(true) {
    double rad = rot * (M_PI / 180);
    /**
     * Matrix inverse to the one above.
     */
    return AffineTransformation(base_type({{std::cos(rad) / xinc, std::sin(rad) / xinc, -(std::sin(rad) * yori + std::cos(rad) * xori) / xinc, -std::sin(rad) / yinc, std::cos(rad) / yinc, (std::sin(rad) * xori - std::cos(rad) * yori) / yinc}}));
}

bool Grid::operator==(const Grid& other) const noexcept(true) {
    return this->m_transformation == other.m_transformation;
}

bool BoundedGrid::operator==(const BoundedGrid& other) const noexcept(true) {
    return Grid::operator==(other) && this->m_nrows == other.m_nrows && this->m_ncols == other.m_ncols;
}

std::size_t BoundedGrid::row(std::size_t i) const noexcept(false) {
    if (i >= this->size())
        throw std::runtime_error("Index out of range");
    return i / this->ncols();
}

std::size_t BoundedGrid::col(std::size_t i) const noexcept(false) {
    if (i >= this->size())
        throw std::runtime_error("Index out of range");
    return i % this->ncols();
}

Point BoundedGrid::to_cdp(
    std::size_t const row,
    std::size_t const col
) const noexcept(false) {
    if (row >= this->nrows())
        throw std::runtime_error("Row out of range");
    if (col >= this->ncols())
        throw std::runtime_error("Col out of range");

    Point point{static_cast<double>(row), static_cast<double>(col)};

    return this->m_transformation * point;
}

Point BoundedGrid::to_cdp(
    std::size_t i
) const noexcept(false) {
    auto row = i / this->ncols();
    auto col = i % this->ncols();

    return to_cdp(row, col);
}

Point BoundedGrid::from_cdp(
    Point point
) const noexcept(false) {
    return this->m_inverse_transformation * point;
}

std::pair<std::size_t, std::size_t> as_pair(std::size_t row, std::size_t col) {
    return std::pair<std::size_t, std::size_t>(row, col);
}

float& RegularSurface::operator[](std::size_t i) noexcept(false) {
    if (i >= this->m_grid.size())
        throw std::runtime_error("operator[]: index out of range");
    return this->m_data[i];
}

const float& RegularSurface::operator[](std::size_t i) const noexcept(false) {
    if (i >= this->m_grid.size())
        throw std::runtime_error("const operator[]: index out of range");
    return this->m_data[i];
}

float& RegularSurface::operator[](std::pair<std::size_t, std::size_t> p) noexcept(false) {
    if (p.first >= this->m_grid.nrows())
        throw std::runtime_error("operator[]: index out of range");
    if (p.second >= this->m_grid.ncols())
        throw std::runtime_error("operator[]: index out of range");
    return this->m_data[p.first * this->m_grid.ncols() + p.second];
}

const float& RegularSurface::operator[](std::pair<std::size_t, std::size_t> p) const noexcept(false) {
    if (p.first >= this->m_grid.nrows())
        throw std::runtime_error("const operator[]: index out of range");
    if (p.second >= this->m_grid.ncols())
        throw std::runtime_error("const operator[]: index out of range");
    return this->m_data[p.first * this->m_grid.ncols() + p.second];
}
