/// @file pareto_front.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <sstream>
#include <vector>

#include "flexman/data_structure/solution.hpp"

namespace flexman
{

/// @brief A Pareto front.
template <typename State, typename Resources>
struct ParetoFront {
    std::vector<Solution<State, Resources>> solutions; ///< The pareto front.
    double step_length;                                ///< The step length for this partial solution.
    unsigned steps_per_iteration;                      ///< The number of simulation steps per iteration.
    unsigned iteration;                                ///< The current iteration.
    double runtime;                                    ///< The total runtime.
};

/// @brief Converts a ParetoFront object to a string representation.
///
/// @tparam State The type representing the system's state.
/// @tparam Resources The type representing the system's resources.
/// @param pareto_front The ParetoFront object to convert to a string.
/// @return A string summarizing the ParetoFront, including runtime, iteration,
/// step length, and steps per iteration.
template <typename State, typename Resources>
std::string to_string(const flexman::ParetoFront<State, Resources> &pareto_front)
{
    std::stringstream ss;
    ss << "    ParetoFront{\n";
    ss << "        step_length         : " << pareto_front.step_length << "\n";
    ss << "        steps_per_iteration : " << pareto_front.steps_per_iteration << "\n";
    ss << "        iteration           : " << pareto_front.iteration << "\n";
    ss << "        runtime             : " << pareto_front.runtime << "\n";
    ss << "        solutions           : \n";
    for (auto &solution : pareto_front.solutions) {
        ss << "            " << solution << "\n";
    }
    ss << "    }\n";
    return ss.str();
}

} // namespace flexman

/// @brief Outputs a ParetoFront object to an output stream.
///
/// @param lhs The output stream to write to.
/// @param rhs The ParetoFront object to output.
/// @return A reference to the output stream.
template <typename State, typename Resources>
std::ostream &operator<<(std::ostream &lhs, const flexman::ParetoFront<State, Resources> &rhs)
{
    return (lhs << flexman::to_string(rhs));
}
