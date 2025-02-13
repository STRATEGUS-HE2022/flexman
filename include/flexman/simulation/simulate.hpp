/// @file simulate.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Provides simulation functions for evaluating mode sequences
/// and state evolution.
///
/// @details
/// This file defines functions for simulating system behavior based on mode
/// execution sequences. It includes:
/// - `generate_solution`: Simulates a sequence of mode executions to generate
///   a resulting solution.
/// - `simulate_one_step`: Performs a single-step simulation update.
/// - `simulate_single_mode`: Simulates a mode for a specified number of steps,
///   tracking state evolution over time.
///
/// These functions allow for controlled execution of mode transitions, enabling
/// efficient solution evaluation in simulation-based optimization processes.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "flexman/search/common.hpp"
#include "flexman/simulation/common.hpp"

namespace flexman
{

/// @brief Provides simulation support functions and structures.
///
/// @details This namespace includes tools for simulating the execution of
/// mode sequences within the system. It enables the evaluation of solutions
/// by applying defined modes iteratively and tracking the evolution of system
/// states and resources over time.
namespace simulation
{

/// @brief Generates a solution by simulating a sequence of mode executions.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the manager that handles solution updates and evaluation.
/// @param modes The vector of modes available for execution.
/// @param sequence The sequence of mode executions to simulate.
///
/// @return The generated solution after simulating the mode sequence.
template <typename State, typename Mode, typename Resources>
auto generate_solution(
    const flexman::core::Manager<State, Mode, Resources> *manager,
    const std::vector<Mode> &modes,
    const std::vector<flexman::core::ModeExecution> &sequence) -> flexman::core::Solution<State, Resources>
{
    // Initialize the solution with an empty state and resources.
    flexman::core::Solution<State, Resources> solution{
        .sequence  = {},
        .state     = manager->initial_state,
        .resources = Resources(),
        .distance  = std::numeric_limits<double>::max(),
    };
    // Simulate the mode sequence.
    for (const auto &mode_execution : sequence) {
        // Apply the mode the specified number of times
        for (std::size_t i = 0; i < mode_execution.times; ++i) {
            // Store the previous solution.
            auto old_solution = solution;
            // Update the solution.
            manager->updated_solution(solution, modes[mode_execution.mode]);
            // Add the mode to the sequence.
            flexman::core::detail::add_mode_execution_to_sequence(mode_execution.mode, solution.sequence);
            // If the solution is complete, interpolate to avoid overshoot.
            if (manager->is_complete(solution)) {
                solution = flexman::search::find_solution_closest_to_zero(manager, old_solution, solution);
                break;
            }
        }
    }
    return solution;
}

/// @brief Simulates one step and updates the solution.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the manager that handles the simulation.
/// @param mode The mode being simulated.
/// @param solution The current solution to update.
template <typename State, typename Mode, typename Resources>
inline void simulate_one_step(
    const flexman::core::Manager<State, Mode, Resources> *manager,
    const Mode &mode,
    flexman::core::Solution<State, Resources> &solution)
{
    // Check for null pointer in manager.
    if (manager == nullptr) {
        throw std::invalid_argument("manager pointer is null");
    }

    // Update the solution using the provided mode.
    manager->updated_solution(solution, mode);
}

/// @brief Simulates the mode and produces a new solution at each step.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the manager handling the simulation.
/// @param mode The mode being simulated.
/// @param steps The number of steps to simulate.
///
/// @return A structure defining default simulation specifications.
template <typename State, typename Mode, typename Resources>
inline auto simulate_single_mode(
    const flexman::core::Manager<State, Mode, Resources> *manager,
    const Mode &mode,
    const unsigned steps) -> Simulation<State, Resources>
{
    // Check for null pointer in manager.
    if (manager == nullptr) {
        throw std::invalid_argument("manager pointer is null");
    }

    // Check if steps is a valid number.
    if (steps == 0) {
        throw std::invalid_argument("steps must be greater than 0");
    }

    // Initialize the simulation vector to store solutions at each step.
    Simulation<State, Resources> simulation{
        .evolution     = {},
        .initial_state = manager->initial_state,
        .target_state  = manager->target_state,
    };

    // Initialize the initial solution with the initial state and empty sequence.
    flexman::core::Solution<State, Resources> solution{
        .sequence  = {},                                 // Empty sequence initially.
        .state     = manager->initial_state,             // Start from the initial state.
        .resources = Resources(),                        // Initialize resources.
        .distance  = std::numeric_limits<double>::max(), // Initialize the distance to maximum.
    };

    // Perform the simulation for the given number of steps, or until the solution is complete.
    for (unsigned i = 0; (i < steps) && !manager->is_complete(solution); ++i) {
        // Simulate one step and update the solution.
        simulate_one_step(manager, mode, solution);

        // Save the current state of the solution.
        simulation.evolution.emplace_back(solution);
    }

    // Return the vector containing all the solutions generated during the simulation.
    return simulation;
}

} // namespace simulation
} // namespace flexman
