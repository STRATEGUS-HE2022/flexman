/// @file solution.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines the `Solution` structure for representing and comparing
/// system states within an optimization process.
///
/// @details
/// This file introduces the `Solution` template structure, which encapsulates
/// a system state and its associated resources within an optimization search.
/// Each `Solution` consists of:
/// - A sequence of executed modes.
/// - The current state of the system.
/// - Accumulated resource usage.
/// - A distance metric indicating proximity to the target state.
///
/// Additionally, the file provides:
/// - Overloaded comparison operators for solution evaluation.
/// - A function to convert a `Solution` instance into a string format.
/// - Overloaded stream output operators for easy logging and debugging.
///
/// The `Solution` structure plays a key role in optimization frameworks,
/// enabling systematic evaluation and tracking of different state transitions.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <vector>

#include <json/json.hpp>

#include "flexman/core/mode_execution.hpp"

namespace flexman
{
namespace core
{

/// @brief Represents a single solution, which may be incomplete.
///
/// @tparam State The type representing the current state.
/// @tparam Resources The type representing the resources used.
template <typename State, typename Resources>
struct Solution {
    /// @brief The sequence of mode executions.
    std::vector<ModeExecution> sequence;
    /// @brief The current state (x).
    State state;
    /// @brief Resources accumulated so far.
    Resources resources;
    /// @brief Distance from the target state.
    double distance{};

    /// @brief Compares two solutions for equality based on their sequences or resources.
    ///
    /// @param lhs The left-hand side solution.
    /// @param rhs The right-hand side solution.
    ///
    /// @return True if the sequences or resources are equal, otherwise false.
    friend auto operator==(
        const flexman::core::Solution<State, Resources> &lhs,
        const flexman::core::Solution<State, Resources> &rhs) noexcept -> bool
    {
        return (lhs.sequence == rhs.sequence) || (lhs.resources == rhs.resources);
    }

    /// @brief Compares two solutions to determine if one is "less than" the other.
    ///
    /// @param lhs The left-hand side solution.
    /// @param rhs The right-hand side solution.
    ///
    /// @return True if the sequences differ and the resources of lhs are less than rhs.
    friend auto operator<(
        const flexman::core::Solution<State, Resources> &lhs,
        const flexman::core::Solution<State, Resources> &rhs) noexcept -> bool
    {
        return (lhs.sequence != rhs.sequence) && (lhs.resources < rhs.resources);
    }

    /// @brief Converts a Solution object to a string representation.
    ///
    /// @return A string summarizing the Solution, including state, resources,
    /// distance, and mode sequence size.
    auto to_string() const -> std::string
    {
        std::stringstream ss;
        ss << "Solution{";
        ss << "distance: " << std::fixed << std::setprecision(3) << std::setw(7) << std::right << distance << ", ";
        ss << "resources: " << resources << ", ";
        ss << "sequence:[ ";
        for (std::size_t i = 0U; i < sequence.size(); ++i) {
            ss << std::setw(2) << std::right << static_cast<unsigned>(sequence[i].mode) << "*" << std::setw(3)
               << std::left << sequence[i].times;
            if (i < sequence.size()) {
                ss << " ";
            }
        }
        ss << " ]}";
        return ss.str();
    }
};

} // namespace core
} // namespace flexman

/// @brief Outputs a Solution object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The Solution object to output.
///
/// @return A reference to the output stream.
template <typename State, typename Resources>
auto operator<<(std::ostream &lhs, const flexman::core::Solution<State, Resources> &rhs) -> std::ostream &
{
    return (lhs << rhs.to_string());
}
