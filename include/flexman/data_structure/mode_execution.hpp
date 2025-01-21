/// @file mode_execution.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include "flexman/data_structure/mode.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

namespace flexman
{

/// @brief Represents the execution of a specific mode.
/// Encapsulates a mode identifier and the number of times the mode is executed.
struct ModeExecution {
    mode_id_t mode;    ///< Identifier of the mode to execute.
    std::size_t times; ///< Number of consecutive executions for the mode.

    /// @brief Constructs a ModeExecution instance.
    /// @param _mode The identifier of the mode.
    /// @param _times The number of times to execute the mode.
    ModeExecution(mode_id_t _mode, std::size_t _times)
        : mode(_mode), times(_times)
    {
        // Nothing to do.
    }

    /// @brief Compares two ModeExecution objects for equality.
    /// @param lhs The left-hand side ModeExecution.
    /// @param rhs The right-hand side ModeExecution.
    /// @return True if both mode and times are equal; false otherwise.
    friend inline bool
    operator==(const ModeExecution &lhs, const ModeExecution &rhs) noexcept
    {
        return (lhs.mode == rhs.mode) && (lhs.times == rhs.times);
    }

    /// @brief Compares two ModeExecution objects for inequality.
    /// @param lhs The left-hand side ModeExecution.
    /// @param rhs The right-hand side ModeExecution.
    /// @return True if either mode or times differ; false otherwise.
    friend inline bool operator!=(const ModeExecution &lhs, const ModeExecution &rhs) noexcept
    {
        return (lhs.mode != rhs.mode) || (lhs.times != rhs.times);
    }
};

/// @brief Adds a mode to the sequence or updates the count of the last mode if it matches.
/// @param mode The mode to execute.
/// @param sequence The sequence of mode executions to update.
inline void add_mode_execution_to_sequence(mode_id_t mode, std::vector<ModeExecution> &sequence)
{
    if (sequence.empty() || (sequence.back().mode != mode)) {
        // Add new mode to the sequence if it's empty or different from the last.
        sequence.emplace_back(ModeExecution{ mode, 1 });
    } else {
        // Increment the count if it's the same as the last mode.
        sequence.back().times++;
    }
}

/// @brief Converts a ModeExecution object to a string representation.
///
/// @param rhs The ModeExecution object to convert to a string.
/// @return A string summarizing the ModeExecution, including mode identifier
/// and the execution count.
std::string to_string(const flexman::ModeExecution &rhs)
{
    std::stringstream ss;
    ss << std::setw(2) << std::right << rhs.mode << "*" << std::setw(3) << std::left << rhs.times;
    return ss.str();
}

} // namespace flexman

/// @brief Outputs a ModeExecution object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The ModeExecution object to output.
/// @return A reference to the output stream, allowing for chaining.
std::ostream &operator<<(std::ostream &lhs, const flexman::ModeExecution &rhs)
{
    return (lhs << flexman::to_string(rhs));
}
