/// @file mode_execution.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Manages the execution tracking of modes and provides utilities for
/// sequence handling.
///
/// @details
/// This file defines the `ModeExecution` structure, which associates a mode
/// identifier with the number of times it has been executed consecutively. It
/// also provides utility functions for managing mode execution sequences,
/// including:
/// - Adding a mode to a sequence while ensuring consecutive executions are
///   counted.
/// - Converting a `ModeExecution` instance to a human-readable string format.
/// - Overloading comparison and stream output operators to facilitate
///   manipulation and logging.
///
/// The utilities in this file are useful for managing control sequences in
/// systems where mode transitions are tracked and optimized.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "flexman/core/mode.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace flexman
{
namespace core
{

/// @brief Represents the execution of a specific mode. Encapsulates a mode
/// identifier and the number of times the mode is executed.
struct ModeExecution {
    /// @brief Identifier of the mode to execute.
    ModeId mode;
    /// @brief Number of consecutive executions for the mode.
    std::size_t times;

    /// @brief Constructs a ModeExecution instance.
    ///
    /// @param _mode The identifier of the mode.
    /// @param _times The number of times to execute the mode.
    ModeExecution(flexman::core::ModeId _mode, std::size_t _times)
        : mode(_mode)
        , times(_times)
    {
        // Nothing to do.
    }

    /// @brief Compares two ModeExecution objects for equality.
    ///
    /// @param lhs The left-hand side ModeExecution.
    /// @param rhs The right-hand side ModeExecution.
    ///
    /// @return True if both mode and times are equal; false otherwise.
    friend auto operator==(const ModeExecution &lhs, const ModeExecution &rhs) noexcept -> bool
    {
        return (lhs.mode == rhs.mode) && (lhs.times == rhs.times);
    }

    /// @brief Compares two ModeExecution objects for inequality.
    ///
    /// @param lhs The left-hand side ModeExecution.
    /// @param rhs The right-hand side ModeExecution.
    ///
    /// @return True if either mode or times differ; false otherwise.
    friend auto operator!=(const ModeExecution &lhs, const ModeExecution &rhs) noexcept -> bool
    {
        return (lhs.mode != rhs.mode) || (lhs.times != rhs.times);
    }

    /// @brief Converts a ModeExecution object to a string representation.
    ///
    /// @return A string summarizing the ModeExecution, including mode identifier
    /// and the execution count.
    auto to_string() const -> std::string
    {
        std::stringstream ss;
        ss << std::setw(2) << std::right << mode << "*" << std::setw(3) << std::left << times;
        return ss.str();
    }
};

/// @brief Support functions.
namespace detail
{
/// @brief Adds a mode to the sequence or updates the count of the last mode if it matches.
///
/// @param mode The mode to execute.
/// @param sequence The sequence of mode executions to update.
inline void add_mode_execution_to_sequence(flexman::core::ModeId mode, std::vector<ModeExecution> &sequence)
{
    if (sequence.empty() || (sequence.back().mode != mode)) {
        // Add new mode to the sequence if it's empty or different from the last.
        sequence.emplace_back(mode, 1);
    } else {
        // Increment the count if it's the same as the last mode.
        sequence.back().times++;
    }
}
} // namespace detail

} // namespace core
} // namespace flexman

/// @brief Outputs a ModeExecution object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The ModeExecution object to output.
///
/// @return A reference to the output stream, allowing for chaining.
auto operator<<(std::ostream &lhs, const flexman::core::ModeExecution &rhs) -> std::ostream &
{
    return (lhs << rhs.to_string());
}
