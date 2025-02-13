/// @file fsmlib_support.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Provides utility functions for serializing and deserializing FSMlib data structures.
///
/// @details
/// This file defines:
/// - String conversion utilities for `fsmlib::Vector` and `fsmlib::Matrix`.
/// - JSON serialization (`operator<<`) and deserialization (`operator>>`)
///   for `fsmlib::Vector`, `fsmlib::Matrix`, `fsmlib::control::StateSpace`,
///   and `fsmlib::control::DiscreteStateSpace`.
///
/// These utilities facilitate the integration of FSMlib data structures with
/// JSON-based data exchange, allowing easy storage and retrieval of system
/// models.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <json/json.hpp>

#include <fsmlib/control.hpp>

namespace detail
{

template <typename T, std::size_t N>
inline std::string to_string(const fsmlib::Vector<T, N> &rhs)
{
    std::stringstream ss;
    ss << "[";
    for (std::size_t i = 0; i < N; ++i) {
        ss << rhs[i];
        if (i < N - 1)
            ss << ", ";
    }
    ss << "]";
    return ss.str();
}

template <typename T, std::size_t Rows, std::size_t Cols>
inline std::string to_string(const fsmlib::Matrix<T, Rows, Cols> &rhs)
{
    std::stringstream ss;
    ss << "[";
    for (std::size_t r = 0; r < Rows; ++r) {
        ss << "[";
        for (std::size_t c = 0; c < Cols; ++c) {
            ss << rhs(r, c);
            if (c < Cols - 1)
                ss << ", ";
        }
        ss << "]";
        if (r < Rows - 1)
            ss << ", ";
    }
    ss << "]";
    return ss.str();
}

} // namespace detail

namespace json
{

template <typename T, std::size_t N>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const fsmlib::Vector<T, N> &rhs)
{
    lhs.clear();
    lhs.set_type(json::JTYPE_ARRAY);
    lhs.resize(N);
    for (std::size_t i = 0; i < N; ++i) {
        lhs[i] << rhs[i];
    }
    return lhs;
}

template <typename T, std::size_t N>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, fsmlib::Vector<T, N> &rhs)
{
    if ((lhs.get_type() == json::JTYPE_ARRAY) && (lhs.size() == N)) {
        for (std::size_t i = 0; i < N; ++i) {
            lhs[i] >> rhs[i];
        }
    }
    return lhs;
}

template <typename T, std::size_t Rows, std::size_t Cols>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const fsmlib::Matrix<T, Rows, Cols> &rhs)
{
    lhs.clear();
    lhs.set_type(json::JTYPE_ARRAY);
    lhs.resize(Rows);
    for (std::size_t r = 0; r < Rows; ++r) {
        lhs[r].set_type(json::JTYPE_ARRAY);
        lhs[r].resize(Cols);
        for (std::size_t c = 0; c < Cols; ++c) {
            lhs[r][c] << rhs(r, c);
        }
    }
    return lhs;
}

template <typename T, std::size_t Rows, std::size_t Cols>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, fsmlib::Matrix<T, Rows, Cols> &rhs)
{
    if ((lhs.get_type() != json::JTYPE_ARRAY) || (lhs.size() != Rows)) {
        return lhs;
    }
    for (std::size_t r = 0; r < Rows; ++r) {
        if ((lhs[r].get_type() != json::JTYPE_ARRAY) || (lhs[r].size() != Cols)) {
            return lhs;
        }
        for (std::size_t c = 0; c < Cols; ++c) {
            lhs[r][c] >> rhs(r, c);
        }
    }
    return lhs;
}

template <typename T, std::size_t Rows, std::size_t Inputs, std::size_t Outputs>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const fsmlib::control::StateSpace<T, Rows, Inputs, Outputs> &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["A"] << rhs.A;
    lhs["B"] << rhs.B;
    lhs["C"] << rhs.C;
    lhs["D"] << rhs.D;
    return lhs;
}

template <typename T, std::size_t Rows, std::size_t Inputs, std::size_t Outputs>
inline const json::jnode_t &
operator>>(const json::jnode_t &lhs, fsmlib::control::StateSpace<T, Rows, Inputs, Outputs> &rhs)
{
    if (lhs.get_type() == json::JTYPE_OBJECT) {
        lhs["A"] >> rhs.A;
        lhs["B"] >> rhs.B;
        lhs["C"] >> rhs.C;
        lhs["D"] >> rhs.D;
    }
    return lhs;
}

template <typename T, std::size_t Rows, std::size_t Inputs, std::size_t Outputs>
inline json::jnode_t &
operator<<(json::jnode_t &lhs, const fsmlib::control::DiscreteStateSpace<T, Rows, Inputs, Outputs> &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["A"] << rhs.A;
    lhs["B"] << rhs.B;
    lhs["C"] << rhs.C;
    lhs["D"] << rhs.D;
    lhs["sample_time"] << rhs.sample_time;
    return lhs;
}

template <typename T, std::size_t Rows, std::size_t Inputs, std::size_t Outputs>
inline const json::jnode_t &
operator>>(const json::jnode_t &lhs, fsmlib::control::DiscreteStateSpace<T, Rows, Inputs, Outputs> &rhs)
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
