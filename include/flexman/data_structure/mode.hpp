/// @file mode.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <sstream>
#include <cstdint>
#include <memory>
#include <vector>

namespace flexman
{

/// @brief Keeps track of the mode.
using mode_id_t = std::size_t;

/// @brief Represents a mode of operation in a system.
/// @tparam System The type defining the system's dynamics.
/// @tparam Input The type defining the system's input.
template <typename System, typename Input>
struct Mode {
    std::size_t id; ///< Unique identifier for the mode.
    System system;  ///< The system's dynamic representation (e.g., state-space matrices).
    Input input;    ///< Fixed input for the mode.

    /// @brief Default virtual destructor for polymorphic use.
    virtual ~Mode() = default;
};

/// @brief Converts a Mode object to a string representation.
///
/// @tparam System The type defining the system's dynamics.
/// @tparam Input The type defining the system's input.
/// @param rhs The Mode object to convert to a string.
/// @return A string containing the unique identifier of the Mode.
template <typename System, typename Input>
std::string to_string(const flexman::Mode<System, Input> &rhs)
{
    std::stringstream ss;
    ss << rhs.id;
    return ss.str();
}


} // namespace flexman

/// @brief Outputs a Mode object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The Mode object to output.
/// @return A reference to the output stream.
template <typename System, typename Input>
std::ostream &operator<<(std::ostream &lhs, const flexman::Mode<System, Input> &rhs)
{
    return (lhs << flexman::to_string(rhs));
}
