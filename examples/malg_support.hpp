/// @file malg_support.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <json/json.hpp>
#include <malg/control/control.hpp>
#include <malg/matrix.hpp>
#include <malg/io.hpp>

namespace detail
{

template <class T>
inline std::string to_string(const malg::Vector<T> &rhs)
{
    std::stringstream ss;
    ss << rhs;
    return ss.str();
}

template <class T>
inline std::string to_string(const malg::Matrix<T> &rhs)
{
    std::stringstream ss;
    ss << rhs;
    return ss.str();
}

} // namespace detail

namespace json
{

template <class T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const malg::Vector<T> &rhs)
{
    lhs.clear();
    lhs.set_type(json::JTYPE_ARRAY);
    lhs.resize(rhs.size());
    for (unsigned i = 0; i < rhs.size(); ++i) {
        lhs[i] << rhs[i];
    }
    return lhs;
}

template <class T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const malg::Matrix<T> &rhs)
{
    lhs.clear();
    lhs.set_type(json::JTYPE_ARRAY);
    lhs.resize(rhs.rows());
    for (unsigned r = 0; r < rhs.rows(); ++r) {
        lhs[r].set_type(json::JTYPE_ARRAY);
        lhs[r].resize(rhs.cols());
        for (unsigned c = 0; c < rhs.cols(); ++c) {
            lhs[r][c] << rhs(r, c);
        }
    }
    return lhs;
}

template <class T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, malg::Vector<T> &rhs)
{
    if ((lhs.get_type() == json::JTYPE_ARRAY) && (lhs.size() > 0)) {
        // Resize the vector.
        rhs.resize(lhs.size());
        // Load the vector.
        for (unsigned i = 0; i < rhs.size(); ++i) {
            lhs[i] >> rhs[i];
        }
    }
    return lhs;
}

template <class T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, malg::Matrix<T> &rhs)
{
    if ((lhs.get_type() != json::JTYPE_ARRAY) || (lhs.size() == 0)) {
        return lhs;
    }
    if ((lhs[0].get_type() != json::JTYPE_ARRAY) || (lhs[0].size() == 0)) {
        return lhs;
    }
    // Resize the matrix.
    rhs.resize(lhs.size(), lhs[0].size());
    // Load the matrix.
    for (unsigned r = 0; r < rhs.rows(); ++r) {
        // Check the number of elements.
        assert(lhs[r].size() == rhs.cols());
        // Load the row.
        for (unsigned c = 0; c < rhs.cols(); ++c) {
            lhs[r][c] >> rhs(r, c);
        }
    }
    return lhs;
}

template <class T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const malg::control::StateSpace<T> &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["A"] << rhs.A;
    lhs["B"] << rhs.B;
    lhs["C"] << rhs.C;
    lhs["D"] << rhs.D;
    return lhs;
}

template <class T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, malg::control::StateSpace<T> &rhs)
{
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        lhs["A"] >> rhs.A;
        lhs["B"] >> rhs.B;
        lhs["C"] >> rhs.C;
        lhs["D"] >> rhs.D;
    }
    return lhs;
}

template <class T>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const malg::control::DiscreteStateSpace<T> &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["A"] << rhs.A;
    lhs["B"] << rhs.B;
    lhs["C"] << rhs.C;
    lhs["D"] << rhs.D;
    lhs["sample_time"] << rhs.sample_time;
    return lhs;
}

template <class T>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, malg::control::DiscreteStateSpace<T> &rhs)
{
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        lhs["A"] >> rhs.A;
        lhs["B"] >> rhs.B;
        lhs["C"] >> rhs.C;
        lhs["D"] >> rhs.D;
        lhs["sample_time"] >> rhs.sample_time;
    }
    return lhs;
}

} // namespace json
