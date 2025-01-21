/// @file solution.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <vector>

#include <json/json.hpp>

#include "flexman/data_structure/mode_execution.hpp"

namespace flexman
{

/// @brief Represents a single solution, which may be incomplete.
///
/// @tparam State The type representing the current state.
/// @tparam Resources The type representing the resources used.
template <typename State, typename Resources>
struct Solution {
    std::vector<ModeExecution> sequence; ///< The sequence of mode executions.
    State state;                         ///< The current state (x).
    Resources resources;                 ///< Resources accumulated so far.
    double distance;                     ///< Distance from the target state.

    /// @brief Compares two solutions for equality based on their sequences or resources.
    ///
    /// @param lhs The left-hand side solution.
    /// @param rhs The right-hand side solution.
    ///
    /// @return True if the sequences or resources are equal, otherwise false.
    friend inline bool operator==(const flexman::Solution<State, Resources> &lhs, const flexman::Solution<State, Resources> &rhs) noexcept
    {
        return (lhs.sequence == rhs.sequence) || (lhs.resources == rhs.resources);
    }

    /// @brief Compares two solutions to determine if one is "less than" the other.
    ///
    /// @param lhs The left-hand side solution.
    /// @param rhs The right-hand side solution.
    ///
    /// @return True if the sequences differ and the resources of lhs are less than rhs.
    friend inline bool operator<(const flexman::Solution<State, Resources> &lhs, const flexman::Solution<State, Resources> &rhs) noexcept
    {
        return (lhs.sequence != rhs.sequence) && (lhs.resources < rhs.resources);
    }
};

/// @brief Converts a Solution object to a string representation.
///
/// @tparam State The type representing the system's state.
/// @tparam Resources The type representing the system's resources.
/// @param rhs The Solution object to convert to a string.
/// @return A string summarizing the Solution, including state, resources,
/// distance, and mode sequence size.
template <typename State, typename Resources>
inline std::string to_string(const flexman::Solution<State, Resources> &rhs)
{
    std::stringstream ss;
    ss << "Solution{";
    ss << "distance: " << std::fixed << std::setprecision(3) << std::setw(7) << std::right << rhs.distance << ", ";
    ss << "resources: " << rhs.resources << ", ";
    ss << "sequence:[ ";
    for (std::size_t i = 0u; i < rhs.sequence.size(); ++i) {
        ss << std::setw(2) << std::right << static_cast<unsigned>(rhs.sequence[i].mode) << "*"
           << std::setw(3) << std::left << rhs.sequence[i].times;
        if (i < rhs.sequence.size()) {
            ss << " ";
        }
    }
    ss << " ]}";
    return ss.str();
}

} // namespace flexman

/// @brief Outputs a Solution object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The Solution object to output.
/// @return A reference to the output stream.
template <typename State, typename Resources>
std::ostream &operator<<(std::ostream &lhs, const flexman::Solution<State, Resources> &rhs)
{
    return (lhs << flexman::to_string(rhs));
}
