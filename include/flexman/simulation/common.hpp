/// @file common.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines common simulation functions and data structures.
///
/// @details
/// This file provides common components for simulation processes, including:
/// - The `Simulation` structure, which encapsulates the simulation state,
///   including the sequence of mode transitions, the initial state, and
///   the target state.
///
/// These components serve as the foundation for executing and managing
/// simulations, tracking the evolution of states over time, and evaluating
/// the feasibility of reaching target conditions.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "flexman/core/mode.hpp"
#include "flexman/core/solution.hpp"

namespace flexman
{
namespace simulation
{

/// @brief A structure that keeps track of a simulation information.
///
/// @tparam State The type representing the system's state.
/// @tparam Resources The type representing the system's resources.
template <typename State, typename Resources>
struct Simulation {
    /// @brief Sequence of modes used in the simulation.
    std::vector<flexman::core::Solution<State, Resources>> evolution;
    /// @brief Initial state of the simulation.
    State initial_state;
    /// @brief The target state to reach during the simulation.
    State target_state;
};

} // namespace simulation
} // namespace flexman
