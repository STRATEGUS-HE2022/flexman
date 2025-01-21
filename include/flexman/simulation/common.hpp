/// @file common.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Common simulation functions.

#pragma once

#include "flexman/data_structure/solution.hpp"
#include "flexman/data_structure/mode.hpp"

namespace flexman::simulation
{

/// @brief A structure defining default simulation specifications.
/// @tparam State The type representing the simulation state.
template <typename State, typename Resources>
struct Simulation {
    /// @brief Sequence of modes used in the simulation.
    std::vector<Solution<State, Resources>> evolution;
    /// @brief Initial state of the simulation.
    State initial_state;
    /// @brief The target state to reach during the simulation.
    State target_state;
};

} // namespace flexman::simulation
