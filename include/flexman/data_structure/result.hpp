/// @file result.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <sstream>

#include "flexman/data_structure/pareto_front.hpp"

namespace flexman
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
};

/// @brief Converts a Result object to a string representation.
///
/// @tparam State The type representing the system's state.
/// @tparam Resources The type representing the system's resources.
/// @param result The Result object to convert to a string.
/// @return A string summarizing the Result, including total runtime and
/// the number of Pareto fronts.
template <typename State, typename Resources>
std::string to_string(const flexman::Result<State, Resources> &result)
{
    std::stringstream ss;
    ss << "Result{\n";
    ss << "    runtime : " << result.get_total_runtime() << "\n";
    ss << "    pareto_fronts : \n";
    for (auto &pareto_front : result.pareto_fronts) {
        ss << pareto_front;
    }
    ss << "}\n";
    return ss.str();
}

} // namespace flexman

/// @brief Outputs a Result object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The Result object to output.
/// @return A reference to the output stream.
template <typename State, typename Resources>
std::ostream &operator<<(std::ostream &lhs, const flexman::Result<State, Resources> &rhs)
{
    return (lhs << flexman::to_string(rhs));
}
