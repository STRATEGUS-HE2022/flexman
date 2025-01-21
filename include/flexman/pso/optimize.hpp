/// @file optimize.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Main PSO search functions.

#pragma once

#include "flexman/data_structure/manager.hpp"
#include "flexman/data_structure/result.hpp"
#include "flexman/simulation/simulate.hpp"
#include "flexman/pso/common.hpp"

namespace flexman::pso
{

/// @brief Optimizes a solution using the provided manager and solver parameters.
///
/// @tparam State The type representing the system's state.
/// @tparam Mode The type representing the system's mode.
/// @tparam Resources The type representing the system's resources.
/// @param manager Pointer to the manager handling the optimization process.
/// @param parameters The solver parameters to guide the optimization.
/// @param modes A vector of modes available for the optimization process.
/// @param initial_solution The initial solution to refine and optimize.
/// @return The optimized solution.
template <typename State, typename Mode, typename Resources>
auto optimize_solution(
    const flexman::Manager<State, Mode, Resources> *manager,
    const SolverParameters &parameters,
    const std::vector<Mode> &modes,
    const flexman::Solution<State, Resources> &initial_solution)
{
    // Particles as ModeExecution sequences.
    std::vector<std::vector<flexman::ModeExecution>> particles(parameters.num_particles);
    std::vector<std::vector<flexman::ModeExecution>> personal_best(parameters.num_particles);
    std::vector<std::vector<double>> velocities(parameters.num_particles);

    std::vector<double> personal_best_fitness(parameters.num_particles, std::numeric_limits<double>::max());

    std::vector<flexman::ModeExecution> global_best = initial_solution.sequence;
    double global_best_fitness                      = initial_solution.resources.energy + initial_solution.resources.time;

    // Initialize particles
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(1, 10); // Random initial execution times between 1 and 10

    for (size_t i = 0; i < parameters.num_particles; ++i) {
        particles[i]     = initial_solution.sequence;   // Initialize particles with initial solution
        personal_best[i] = particles[i];                // Copy particles into personal_best
        velocities[i].resize(particles[i].size(), 0.0); // Match velocity size to particle size

        for (auto &mode_exec : particles[i]) {
            mode_exec.times = dist(gen); // Randomize execution times
        }
    }
    global_best = personal_best[0]; // Initialize global_best to the first particle

    // Initialize velocities
    velocities.resize(parameters.num_particles);
    for (auto &velocity : velocities) {
        velocity.resize(particles[0].size(), 0.0); // Initialize all velocities to 0.0
    }

    // Main PSO loop
    for (size_t iteration = 0; iteration < parameters.max_iterations; ++iteration) {
        size_t valid_solution_count = 0;

        for (size_t i = 0; i < parameters.num_particles; ++i) {
            // Generate solution from the current particle.
            auto solution = flexman::simulation::generate_solution(manager, modes, particles[i]);

            // Check if the solution is valid
            bool valid_solution = manager->is_complete(solution);

            valid_solution_count += valid_solution;

            // Evaluate fitness.
            double fitness;
            if (valid_solution) {
                // Prioritize minimizing resources for valid solutions
                fitness = solution.resources.energy + solution.resources.time;
            } else {
                // Penalize invalid solutions
                fitness = std::numeric_limits<double>::max();
            }

            // Update personal best
            if (fitness < personal_best_fitness[i]) {
                personal_best[i]         = particles[i];
                personal_best_fitness[i] = fitness;
            }

            // Update global best
            if (fitness < global_best_fitness) {
                global_best         = particles[i];
                global_best_fitness = fitness;
            }
        }

        // Print progress
        qinfo(logging::pso,
              "        Iteration %2u/%2u, best fitness: %6.2f, valid solutions: %3u/%3u\r",
              iteration + 1,
              parameters.max_iterations,
              global_best_fitness,
              valid_solution_count,
              parameters.num_particles);

        // Update velocities and positions
        for (size_t i = 0; i < parameters.num_particles; ++i) {
            for (size_t j = 0; j < particles[i].size(); ++j) {
                // Update velocity using PSO formula
                velocities[i][j] = parameters.inertia * velocities[i][j] +
                                   parameters.cognitive * (static_cast<double>(personal_best[i][j].times) - static_cast<double>(particles[i][j].times)) +
                                   parameters.social * (static_cast<double>(global_best[j].times) - static_cast<double>(particles[i][j].times));

                // Update particle position (execution times)
                double updated_times = static_cast<double>(particles[i][j].times) + velocities[i][j];

                // Clamp updated times to valid range [1, 100] for execution counts
                particles[i][j].times = static_cast<std::size_t>(std::clamp(updated_times, 1.0, 100.0));
            }
        }
    }

    // Print progress
    qinfo(logging::pso, "\n");

    return flexman::simulation::generate_solution(manager, modes, global_best);
}

/// @brief Optimizes a Pareto front using the provided manager and solver parameters.
///
/// @tparam State The type representing the system's state.
/// @tparam Mode The type representing the system's mode.
/// @tparam Resources The type representing the system's resources.
/// @param manager Pointer to the manager handling the optimization process.
/// @param parameters The solver parameters to guide the optimization.
/// @param modes A vector of modes available for the optimization process.
/// @param pareto_front The Pareto front to refine and optimize.
/// @return The optimized Pareto front.
template <typename State, typename Mode, typename Resources>
auto optimize_pareto_front(
    const flexman::Manager<State, Mode, Resources> *manager,
    const SolverParameters &parameters,
    const std::vector<Mode> &modes,
    const flexman::ParetoFront<State, Resources> &pareto_front)
{
    flexman::ParetoFront<State, Resources> optimized = {
        .solutions           = {},
        .step_length         = pareto_front.step_length,
        .steps_per_iteration = pareto_front.steps_per_iteration,
        .iteration           = pareto_front.iteration,
        .runtime             = pareto_front.runtime,
    };

    size_t index = 1, total = pareto_front.solutions.size();
    for (const auto &solution : pareto_front.solutions) {
        qinfo(logging::pso, "    Optimize solution %3u/%3u...\n", index++, total);
        optimized.solutions.emplace_back(
            optimize_solution(
                manager,
                parameters,
                modes,
                solution));
    }
    return optimized;
}

/// @brief Optimizes a result object using the provided manager and solver parameters.
///
/// @tparam State The type representing the system's state.
/// @tparam Mode The type representing the system's mode.
/// @tparam Resources The type representing the system's resources.
/// @param manager Pointer to the manager handling the optimization process.
/// @param parameters The solver parameters to guide the optimization.
/// @param modes A vector of modes available for the optimization process.
/// @param result The result object to refine and optimize.
/// @return The optimized result.
template <typename State, typename Mode, typename Resources>
auto optimize_result(
    const flexman::Manager<State, Mode, Resources> *manager,
    const SolverParameters &parameters,
    const std::vector<Mode> &modes,
    const flexman::Result<State, Resources> &result)
{
    flexman::Result<State, Resources> optimized = {
        .pareto_fronts = {},
    };

    size_t index = 1, total = result.pareto_fronts.size();
    for (const auto &pareto_front : result.pareto_fronts) {
        qinfo(logging::pso, "Optimize Pareto front (step: %6.2f) %3u/%3u...\n",
              pareto_front.step_length, index++, total);
        optimized.pareto_fronts.emplace_back(
            optimize_pareto_front(
                manager,
                parameters,
                modes,
                pareto_front));
    }
    return optimized;
}

} // namespace flexman::pso
