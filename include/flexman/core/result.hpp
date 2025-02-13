/// @file result.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines the `Result` structure for storing simulation outcomes
/// and Pareto-optimal solutions.
///
/// @details
/// This file introduces the `Result` template structure, which aggregates
/// a collection of `ParetoFront` instances, representing sets of non-dominated
/// solutions in a multi-objective optimization problem. It provides:
/// - A method to compute the total runtime across all Pareto fronts.
/// - A function to convert a `Result` instance into a string format.
/// - Overloaded stream output operators for easy logging and debugging.
///
/// The `Result` structure is essential for handling and analyzing the outcome
/// of optimization-based simulations, particularly when managing trade-offs
/// between competing objectives.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <sstream>

#include "flexman/core/pareto_front.hpp"

namespace flexman
{
namespace core
{

/// @brief Represents a simulation result, containing a set of Pareto fronts.
///
/// @tparam State The type representing the system's state.
/// @tparam Resources The type representing the system's resources.
template <typename State, typename Resources>
struct Result {
    std::vector<ParetoFront<State, Resources>> pareto_fronts; ///< The set of Pareto fronts.

    /// @brief Calculates the total runtime across all Pareto fronts.
    ///
    /// @return The total runtime as a double.
    auto get_total_runtime() const
    {
        typename std::vector<ParetoFront<State, Resources>>::const_iterator cit;
        double total_runtime = 0.0;
        for (cit = pareto_fronts.begin(); cit != pareto_fronts.end(); ++cit) {
            total_runtime += cit->runtime;
        }
        return total_runtime;
    }

    /// @brief Converts a Result object to a string representation.
    ///
    /// @return A string summarizing the Result, including total runtime and
    /// the number of Pareto fronts.
    auto to_string() const -> std::string
    {
        std::stringstream ss;
        ss << "Result{\n";
        ss << "    runtime : " << this->get_total_runtime() << "\n";
        ss << "    pareto_fronts : \n";
        for (auto &pareto_front : pareto_fronts) {
            ss << pareto_front;
        }
        ss << "}\n";
        return ss.str();
    }
};

} // namespace core
} // namespace flexman

/// @brief Outputs a Result object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The Result object to output.
///
/// @return A reference to the output stream.
template <typename State, typename Resources>
auto operator<<(std::ostream &lhs, const flexman::core::Result<State, Resources> &rhs) -> std::ostream &
{
    return (lhs << rhs.to_string());
}
