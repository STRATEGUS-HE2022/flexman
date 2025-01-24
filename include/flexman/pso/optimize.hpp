/// @file optimize.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Main PSO search functions.

#pragma once

#include "flexman/data_structure/manager.hpp"
#include "flexman/data_structure/result.hpp"
#include "flexman/simulation/simulate.hpp"
#include "flexman/pso/common.hpp"

#include <random>

namespace flexman
{
namespace pso
{

/// @brief Initializes a random number generator for particle initialization.
/// @param min_execution_time The minimum execution time for the distribution.
/// @param max_execution_time The maximum execution time for the distribution.
/// @return A tuple containing a random number generator and a uniform integer distribution.
inline auto initialize_random_generator(double min_execution_time, double max_execution_time)
    -> std::tuple<std::mt19937, std::uniform_real_distribution<double>>
{
    // Create a random device for seeding the generator.
    std::random_device rd;
    // Initialize a Mersenne Twister random number generator.
    std::mt19937 gen(rd());
    // Create a uniform integer distribution with the specified range.
    std::uniform_real_distribution<double> dist(min_execution_time, max_execution_time);

    // Return the generator and distribution.
    return { gen, dist };
}

/// @brief Updates the velocity and position (number of executions) of a single
/// particle's mode.
///
/// @details This function adjusts the velocity and the number of executions
/// (`times`) for a given mode in a particle based on the PSO formula, which
/// incorporates inertia, cognitive, and social contributions. The updated
/// number of executions is clamped to a valid range to ensure feasibility.
///
/// @param parameters PSO parameters, including inertia, cognitive, and social weights.
/// @param personal_best The personal best number of executions for the given mode.
/// @param global_best The global best number of executions for the given mode.
/// @param velocity Current velocity associated with the particle's mode.
/// @param particle Current mode in the particle to be updated.
inline void update_particle_velocity_and_position(
    const SolverParameters &parameters,
    const flexman::ModeExecution &personal_best,
    const flexman::ModeExecution &global_best,
    double &velocity,
    flexman::ModeExecution &particle)
{
    // Contribution from the particle's previous velocity (inertia component).
    double inertia_contribution = parameters.inertia * velocity;

    // Contribution from the personal best mode (cognitive component).
    double cognitive_contribution = parameters.cognitive * (static_cast<double>(personal_best.times) - static_cast<double>(particle.times));

    // Contribution from the global best mode (social component).
    double social_contribution = parameters.social * (static_cast<double>(global_best.times) - static_cast<double>(particle.times));

    // Combine all contributions to update the velocity.
    velocity = inertia_contribution + cognitive_contribution + social_contribution;

    // Update the number of executions by adding the updated velocity.
    double updated_times = static_cast<double>(particle.times) + velocity;

    // Clamp the updated number of executions to the valid range [1, ...).
    particle.times = static_cast<std::size_t>(std::max(updated_times, 1.0));
}

/// @brief Updates the velocities and number of executions for all particles in
/// the swarm.
///
/// @details This function iterates through all particles in the swarm, updating
/// their velocities and the number of executions (`times`) for each mode based
/// on the PSO formula. It uses the personal and global best solutions to guide
/// the updates, ensuring particles move toward promising areas of the solution
/// space.
///
/// @param parameters PSO parameters guiding the update, including inertia, cognitive, and social weights.
/// @param personal_best Vector of personal best solutions for each particle.
/// @param global_best Global best solution across all particles.
/// @param velocities Vector of velocities for each particle, representing the rate of change in execution counts.
/// @param particles Vector of particles, where each particle contains a sequence of mode executions to be updated.
inline void update_all_particles(
    const SolverParameters &parameters,
    const std::vector<std::vector<flexman::ModeExecution>> &personal_best,
    const std::vector<flexman::ModeExecution> &global_best,
    std::vector<std::vector<double>> &velocities,
    std::vector<std::vector<flexman::ModeExecution>> &particles)
{
    // Iterate through all particles in the swarm.
    for (std::size_t i = 0; i < particles.size(); ++i) {
        // Iterate through each mode in the current particle.
        for (std::size_t j = 0; j < particles[i].size(); ++j) {
            // Update the velocity and number of executions for the current mode in the particle.
            flexman::pso::update_particle_velocity_and_position(
                parameters,          // PSO parameters (inertia, cognitive, social).
                personal_best[i][j], // Personal best for this mode.
                global_best[j],      // Global best for this mode.
                velocities[i][j],    // Current velocity for this mode.
                particles[i][j]);    // Current mode in the particle.
        }
    }
}

/// @brief Evaluates the fitness of a particle and updates personal and global
/// bests.
///
/// @details This function calculates the fitness of a particle by generating a
/// solution based on the given particle's sequence of mode executions. It
/// checks if the solution is valid (i.e., complete) and evaluates its fitness.
/// If the particle improves upon the current personal or global bests, those
/// bests are updated. The fitness is computed based on the energy and time
/// resources used.
///
/// @tparam State The type representing the system's state.
/// @tparam Mode The type representing the system's mode.
/// @tparam Resources The type representing the system's resources.
///
/// @param manager Pointer to the manager handling the optimization process.
/// @param modes A vector of modes available for the optimization process.
/// @param particle The particle to evaluate (sequence of mode executions).
/// @param personal_best The current personal best sequence of mode executions for the particle.
/// @param personal_best_fitness The fitness value of the particle's personal best.
/// @param global_best The current global best sequence of mode executions across all particles.
/// @param global_best_fitness The fitness value of the global best.
/// @return True if the solution generated by the particle is valid (complete); otherwise, false.
template <typename State, typename Mode, typename Resources>
bool evaluate_particle(
    const flexman::Manager<State, Mode, Resources> *manager,
    const std::vector<Mode> &modes,
    const std::vector<flexman::ModeExecution> &particle,
    std::vector<flexman::ModeExecution> &personal_best,
    std::vector<flexman::ModeExecution> &global_best,
    double &personal_best_fitness,
    double &global_best_fitness)
{
    // Generate a solution from the current particle's sequence of mode executions.
    auto solution = flexman::simulation::generate_solution(manager, modes, particle);

    // Check if the generated solution meets the completion criteria.
    bool valid_solution = manager->is_complete(solution);

    // Evaluate the fitness of the solution.
    double fitness;
    if (valid_solution) {
        // If the solution is valid, calculate fitness by minimizing total resources (energy + time).
        fitness = solution.resources.energy + solution.resources.time;
    } else {
        // If the solution is invalid, assign the maximum possible fitness as a penalty.
        fitness = std::numeric_limits<double>::max();
    }

    // Update the personal best for this particle if the current fitness is better.
    if (fitness < personal_best_fitness) {
        personal_best         = particle; // Update personal best sequence.
        personal_best_fitness = fitness;  // Update personal best fitness value.
    }

    // Update the global best across all particles if the current fitness is better.
    if (fitness < global_best_fitness) {
        global_best         = particle; // Update global best sequence.
        global_best_fitness = fitness;  // Update global best fitness value.
    }

    // Return whether the solution is valid.
    return valid_solution;
}

/// @brief Optimizes a solution using the Particle Swarm Optimization (PSO)
/// algorithm.
///
/// @details This function refines and optimizes the initial solution using PSO.
/// It initializes particles with randomized sequences of mode executions,
/// evaluates their fitness, and iteratively updates their velocities and
/// positions based on personal and global best solutions. The algorithm runs
/// for a specified number of iterations and returns the best optimized
/// solution.
///
/// @tparam State The type representing the system's state.
/// @tparam Mode The type representing the system's mode.
/// @tparam Resources The type representing the system's resources.
///
/// @param manager Pointer to the manager handling the optimization process.
/// @param parameters The solver parameters, including the number of particles,
/// inertia, cognitive/social weights, and the maximum number of iterations.
/// @param modes A vector of modes available for the optimization process.
/// @param initial_solution The initial solution to refine and optimize.
///
/// @return The optimized solution, with its mode execution sequence and resources.
template <typename State, typename Mode, typename Resources>
auto optimize_solution(
    const flexman::Manager<State, Mode, Resources> *manager,
    const SolverParameters &parameters,
    const std::vector<Mode> &modes,
    const flexman::Solution<State, Resources> &initial_solution)
{
    // Initialize containers for particles, personal bests, and velocities.
    std::vector<std::vector<flexman::ModeExecution>> particles(parameters.num_particles);
    std::vector<std::vector<flexman::ModeExecution>> personal_best(parameters.num_particles);
    std::vector<std::vector<double>> velocities(parameters.num_particles);

    // Initialize personal and global best fitness values.
    std::vector<double> personal_best_fitness(parameters.num_particles, std::numeric_limits<double>::max());
    std::vector<flexman::ModeExecution> global_best = initial_solution.sequence;
    double global_best_fitness                      = initial_solution.resources.energy + initial_solution.resources.time;

    // Initialize the random number generator and distribution for execution counts.
    auto [gen, dist] = initialize_random_generator(1.0, 10.0);

    // Particle Initialization:
    for (std::size_t i = 0; i < parameters.num_particles; ++i) {
        // Copy the initial solution's sequence to the current particle.
        particles[i] = initial_solution.sequence;

        // Set the personal best for this particle to its initial sequence.
        personal_best[i] = particles[i];

        // Initialize velocities for the particle's modes to zero.
        velocities[i].resize(particles[i].size(), 0.0);

        // Randomize the number of executions (`times`) for each mode in the particle.
        for (auto &mode_exec : particles[i]) {
            // Add randomness while retaining structure.
            mode_exec.times = static_cast<std::size_t>(std::max(static_cast<double>(mode_exec.times) + dist(gen) - 5.0, 1.0));
        }
    }

    // Initialize the global best to the first particle's personal best.
    global_best = personal_best[0];

    // Main PSO Loop:
    for (std::size_t iteration = 0; iteration < parameters.max_iterations; ++iteration) {
        // Count of valid solutions in this iteration.
        std::size_t valid_solution_count = 0;

        // Evaluate each particle's fitness and update personal/global bests.
        for (std::size_t i = 0; i < parameters.num_particles; ++i) {
            valid_solution_count += flexman::pso::evaluate_particle(
                manager,                  // Optimization manager for solution validation.
                modes,                    // Available modes for optimization.
                particles[i],             // Current particle's sequence of mode executions.
                personal_best[i],         // Personal best sequence for this particle.
                global_best,              // Global best sequence across all particles.
                personal_best_fitness[i], // Fitness value of the particle's personal best.
                global_best_fitness);     // Fitness value of the global best.
        }

        // Update velocities and positions of all particles.
        flexman::pso::update_all_particles(
            parameters,    // PSO parameters guiding the update.
            personal_best, // Personal best solutions for all particles.
            global_best,   // Global best solution across the swarm.
            velocities,    // Velocities of the particles.
            particles);    // Current particles being updated.

        // Print the progress of the PSO process.
        qinfo(logging::pso,
              "        Iteration %2u/%2u, best fitness: %6.2f, valid solutions: %3u/%3u\r",
              iteration + 1,
              parameters.max_iterations,
              global_best_fitness,
              valid_solution_count,
              parameters.num_particles);
    }

    // Move to the next line in the output after progress updates.
    qinfo(logging::pso, "\n");

    // Generate and return the optimized solution based on the global best particle.
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

    std::size_t index = 1, total = pareto_front.solutions.size();
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

    std::size_t index = 1, total = result.pareto_fronts.size();
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

} // namespace pso

} // namespace flexman