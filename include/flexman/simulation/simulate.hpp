/// @file simulate.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include "flexman/simulation/common.hpp"
#include "flexman/search/common.hpp"

namespace flexman::simulation
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
Solution<State, Resources> generate_solution(
    const flexman::Manager<State, Mode, Resources> *manager,
    const std::vector<Mode> &modes,
    const std::vector<flexman::ModeExecution> &sequence)
{
    // Initialize the solution with an empty state and resources.
    Solution<State, Resources> solution{
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
            flexman::add_mode_execution_to_sequence(mode_execution.mode, solution.sequence);
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
    const flexman::Manager<State, Mode, Resources> *manager,
    const Mode &mode,
    Solution<State, Resources> &solution)
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
/// @return A vector containing the solution at each step.
template <typename State, typename Mode, typename Resources>
inline auto simulate_single_mode(
    const flexman::Manager<State, Mode, Resources> *manager,
    const Mode &mode,
    const unsigned steps)
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
    Solution<State, Resources> solution{
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

} // namespace flexman::simulation
