/// @file pareto_front.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines the `ParetoFront` structure for managing sets of optimal solutions.
///
/// @details
/// This file introduces the `ParetoFront` template structure, which maintains
/// a set of non-dominated solutions in multi-objective optimization problems.
/// It stores key parameters such as:
/// - A collection of optimal solutions.
/// - The step length used in the optimization process.
/// - The number of steps per iteration.
/// - The current iteration number.
/// - The total runtime of the optimization.
///
/// Additionally, the file provides:
/// - A function to convert a `ParetoFront` instance into a string format.
/// - Overloaded stream output operators for logging and debugging.
///
/// The `ParetoFront` structure is fundamental for algorithms that explore
/// trade-offs between competing objectives, ensuring only non-dominated
/// solutions are retained.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <sstream>
#include <vector>

#include "flexman/core/solution.hpp"

namespace flexman
{
namespace core
{

/// @brief A Pareto front.
///
/// @tparam System The type defining the system's dynamics.
/// @tparam Input The type defining the system's input.
template <typename State, typename Resources>
struct ParetoFront {
    /// @brief The pareto front.
    std::vector<flexman::core::Solution<State, Resources>> solutions;
    /// @brief The step length for this partial solution.
    double step_length;
    /// @brief The number of simulation steps per iteration.
    unsigned steps_per_iteration;
    /// @brief The current iteration.
    unsigned iteration;
    /// @brief The total runtime.
    double runtime;

    /// @brief Converts a ParetoFront object to a string representation.
    ///
    /// @return A string summarizing the ParetoFront, including runtime, iteration,
    /// step length, and steps per iteration.
    auto to_string() const -> std::string
    {
        std::stringstream ss;
        ss << "    ParetoFront{\n";
        ss << "        step_length         : " << step_length << "\n";
        ss << "        steps_per_iteration : " << steps_per_iteration << "\n";
        ss << "        iteration           : " << iteration << "\n";
        ss << "        runtime             : " << runtime << "\n";
        ss << "        solutions           : \n";
        for (auto &solution : solutions) {
            ss << "            " << solution << "\n";
        }
        ss << "    }\n";
        return ss.str();
    }
};

} // namespace core
} // namespace flexman

/// @brief Outputs a ParetoFront object to an output stream.
/// @param lhs The output stream to write to.
/// @param rhs The ParetoFront object to output.
/// @return A reference to the output stream.
template <typename State, typename Resources>
auto operator<<(std::ostream &lhs, const flexman::core::ParetoFront<State, Resources> &rhs) -> std::ostream &
{
    return (lhs << rhs.to_string());
}
