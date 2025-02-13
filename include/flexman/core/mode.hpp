/// @file mode.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines the `Mode` structure for representing operational modes
/// within a system.
///
/// @details
/// This file introduces the `Mode` template structure, which encapsulates
/// an operational mode in a system. Each mode is uniquely identified and
/// consists of:
/// - A system representation (e.g., state-space dynamics).
/// - A fixed input associated with the mode.
///
/// Additionally, the file provides:
/// - A function to convert a `Mode` instance into a string format.
/// - Overloaded stream output operators for easy logging and debugging.
///
/// The `Mode` structure is fundamental for systems where multiple discrete
/// operational states need to be tracked and managed.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <cstdint>
#include <memory>
#include <sstream>
#include <vector>

namespace flexman
{
namespace core
{

/// @brief Keeps track of the mode.
using ModeId = std::size_t;

/// @brief Represents a mode of operation in a system.
///
/// @tparam System The type defining the system's dynamics.
/// @tparam Input The type defining the system's input.
template <typename System, typename Input>
struct Mode {
    /// @brief Unique identifier for the mode.
    std::size_t id{};
    /// @brief The system's dynamic representation (e.g., state-space matrices).
    System system;
    /// @brief Fixed input for the mode.
    Input input;

    /// @brief Default constructor.
    Mode() = default;

    /// @brief Copy constructor.
    ///
    /// @param other the other instance to copy.
    Mode(const Mode &other) = default;

    /// @brief Copy assignment operator.
    ///
    /// @param other the other instance to copy.
    ///
    /// @return a reference to this instance.
    auto operator=(const Mode &other) -> Mode & = default;

    /// @brief Move constructor.
    ///
    /// @param other the other instance to move.
    Mode(Mode &&other) noexcept = default;

    /// @brief Move assignment operator.
    ///
    /// @param other the other instance to move.
    ///
    /// @return a reference to this instance.
    auto operator=(Mode &&other) noexcept -> Mode & = default;

    /// @brief Default virtual destructor for polymorphic use.
    virtual ~Mode() = default;

    /// @brief Converts a Mode object to a string representation.
    ///
    /// @return A string containing the unique identifier of the Mode.
    auto to_string() const -> std::string
    {
        std::stringstream ss;
        ss << id;
        return ss.str();
    }
};

} // namespace core
} // namespace flexman

/// @brief Outputs a Mode object to an output stream.
///
/// @tparam System The type defining the system's dynamics.
/// @tparam Input The type defining the system's input.
///
/// @param lhs The output stream to write to.
/// @param rhs The Mode object to output.
///
/// @return A reference to the output stream.
template <typename System, typename Input>
auto operator<<(std::ostream &lhs, const flexman::core::Mode<System, Input> &rhs) -> std::ostream &
{
    return (lhs << rhs.to_string());
}
