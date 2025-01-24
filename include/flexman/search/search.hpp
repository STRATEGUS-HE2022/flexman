/// @file search.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Main search functions.

#pragma once

#include "flexman/data_structure/mode.hpp"
#include "flexman/data_structure/result.hpp"
#include "flexman/data_structure/solution.hpp"
#include "flexman/search/common.hpp"
#include "flexman/logging.hpp"

#include <timelib/timer.hpp>
#include <algorithm>
#include <cmath>

namespace flexman::search
{

/// @brief Performs a single iteration of the search process.
///
/// @tparam Algorithm The search algorithm to use.
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the manager handling the search process.
/// @param modes The set of modes available for simulation.
/// @param steps_per_iteration The number of steps simulated in this iteration.
/// @param partial_solutions The set of partial solutions to extend.
/// @param accepted_solutions The set of accepted solutions (Pareto front).
/// @param global_timer The global timer to track the search process duration.
template <SearchAlgorithm Algorithm, typename State, typename Mode, typename Resources>
void perform_search_single_iteration(
    const flexman::Manager<State, Mode, Resources> *manager,
    const std::vector<Mode> &modes,
    const unsigned steps_per_iteration,
    std::vector<Solution<State, Resources>> &partial_solutions,
    std::vector<Solution<State, Resources>> &accepted_solutions,
    const timelib::Timer &global_timer)
{
    // Check if manager is a valid pointer.
    if (!manager) {
        throw std::invalid_argument("manager pointer is null");
    }

    // Check if steps_per_iteration is a positive number.
    if (steps_per_iteration == 0) {
        throw std::invalid_argument("steps_per_iteration must be greater than 0");
    }

    // Check if modes vector is not empty.
    if (modes.empty()) {
        throw std::invalid_argument("modes vector is empty");
    }

    std::vector<Solution<State, Resources>> complete, partial, extended;

    // First, we need t extend the partial solutions we have.
    if (Algorithm == SearchAlgorithm::SingleMachine) {
        extended = extend_solutions<SwitchingMode::None>(manager, modes, steps_per_iteration, partial_solutions, global_timer);
    } else {
        extended = extend_solutions<SwitchingMode::Free>(manager, modes, steps_per_iteration, partial_solutions, global_timer);
    }
    flexman::search::log_solutions(logging::solution, quire::debug, extended);

    // Remove the dominated solutions from the Pareto front.
    flexman::search::remove_dominated_solutions<SearchAlgorithm::Exhaustive>(manager, extended, accepted_solutions);
    flexman::search::log_solutions(logging::solution, quire::debug, extended);

    // We split between complete solutions and partial ones.
    flexman::search::split_complete_partial(manager, extended, complete, partial);

    // We need to save complete solutions.
    if (!complete.empty()) {
        // Move solutions from `complete` to `accepted_solutions`.
        flexman::search::move_elements(complete, accepted_solutions);
        // Remove dominated solutions.
        flexman::search::remove_dominated_solutions<SearchAlgorithm::Exhaustive>(manager, accepted_solutions);
        // Then we need to remove duplicate solutions.
        flexman::search::remove_duplicate_solutions(accepted_solutions);
    }

    // Apply heuristic.
    if constexpr (Algorithm == SearchAlgorithm::Heuristic) {
        // Copy the list of partial solutions.
        partial_solutions = partial;
        flexman::search::remove_dominated_solutions<SearchAlgorithm::Heuristic>(manager, partial_solutions, partial);
    } else {
        // Update the list of partial solutions.
        partial_solutions = std::move(partial);
    }
}

/// @brief Performs multiple iterations of the search process.
///
/// @tparam Algorithm The search algorithm to use.
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the manager handling the search process.
/// @param modes The set of modes available for simulation.
/// @param steps_per_iteration The number of steps simulated per iteration.
/// @param previous_pareto_front The previous Pareto front of solutions.
/// @param global_timer The global timer to track the search process duration.
///
/// @return The updated Pareto front after performing the iterations.
template <SearchAlgorithm Algorithm, typename State, typename Mode, typename Resources>
auto perform_search_n_iterations(
    const flexman::Manager<State, Mode, Resources> *manager,
    const std::vector<Mode> &modes,
    const unsigned steps_per_iteration,
    const ParetoFront<State, Resources> &previous_pareto_front,
    const timelib::Timer &global_timer)
{
    // Check if manager is a valid pointer.
    if (!manager) {
        throw std::invalid_argument("manager pointer is null");
    }

    // Check if steps_per_iteration is a positive number.
    if (steps_per_iteration == 0) {
        throw std::invalid_argument("steps_per_iteration must be greater than 0");
    }

    // Check if total_iterations is a positive number.
    if (steps_per_iteration == 0) {
        throw std::invalid_argument("total_iterations must be greater than 0");
    }

    // Check if modes vector is not empty.
    if (modes.empty()) {
        throw std::invalid_argument("modes vector is empty");
    }

    // Prepare the initial partial solutions.
    std::vector<Solution<State, Resources>> partial_solutions;
    // Iterate over the modes.
    for (const auto &mode : modes) {
        partial_solutions.push_back(
            // Initial solution.
            Solution<State, Resources>{
                .sequence  = { { mode.id, 0 } },                 // Empty sequence initially.
                .state     = manager->initial_state,             // Start from the initial state.
                .resources = Resources(),                        // Initialize resources.
                .distance  = std::numeric_limits<double>::max(), // Initialize the distance to maximum.
            });
    }

    // Prepare the accepted solutions from the previous Pareto front.
    std::vector<Solution<State, Resources>> accepted_solutions = previous_pareto_front.solutions;

    // A stopwatch to check runtime.
    timelib::Timer pareto_timer, round_timer;

    // Start the pareto timer.
    pareto_timer.start();

    // Calculate the time covered in each iteration.
    const double time_per_iteration = manager->time_delta * static_cast<double>(steps_per_iteration);

    // Determine the maximum number of steps allowed.
    const unsigned max_iterations = static_cast<unsigned>(manager->time_max / time_per_iteration);

    qinfo(logging::round, "\nPerform %6u iterations maximum, with %5u steps per iteration, each simulating %7.2f.\n",
          max_iterations, steps_per_iteration, time_per_iteration);

    // Perform the search for the specified number of steps or until no partial solutions remain.
    unsigned iteration = 0;
    while ((iteration < max_iterations) && !partial_solutions.empty()) {
        // Start the round timer.
        round_timer.start();

        // Perform a single iteration of the search process.
        flexman::search::perform_search_single_iteration<Algorithm>(
            manager,
            modes,
            steps_per_iteration,
            partial_solutions,
            accepted_solutions,
            global_timer);

        ++iteration;

        qinfo(logging::round, "Step: %6d/%-6d, ", iteration, max_iterations);
        qinfo(logging::round, "Part: %6d, ", partial_solutions.size());
        qinfo(logging::round, "Full: %6d, ", accepted_solutions.size());
        qinfo(logging::round, "RndTm: %8.3f s, ", round_timer.elapsed().count());
        qinfo(logging::round, "RunTm: %8.3f s , ", global_timer.elapsed().count());
        qinfo(logging::round, "RemTm: %8.3f s\r", global_timer.remaining().count());
        if ((iteration == max_iterations) || partial_solutions.empty()) {
            qinfo(logging::round, "\n");
        }

        qdebug(logging::solution, "Accepted solutions:\n");
        flexman::search::log_solutions(logging::solution, quire::debug, accepted_solutions);
        qdebug(logging::solution, "Partial solutions:\n");
        flexman::search::log_solutions(logging::solution, quire::debug, partial_solutions);

        if (global_timer.has_timeout()) {
            qwarning(logging::round, "Iteration index %2u of %3u (Steps: %d, Length: %.2f), went into timeout (%.2f > %.2f).\n",
                     iteration,
                     max_iterations,
                     steps_per_iteration,
                     time_per_iteration,
                     global_timer.elapsed().count(),
                     manager->timeout.count());
            break;
        }
    }

    auto new_pareto_front = ParetoFront<State, Resources>{
        .solutions           = accepted_solutions,            // The final set of accepted solutions.
        .step_length         = time_per_iteration,            // The length of each iteration.
        .steps_per_iteration = steps_per_iteration,           // The number of steps per iteration.
        .iteration           = iteration,                     // The total number of iterations performed.
        .runtime             = pareto_timer.elapsed().count() // The runtime of the search process.
    };

    // Return the updated Pareto front after performing the iterations.
    return new_pareto_front;
}

/// @brief Performs a search using the given parameters and modes.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the manager handling the search process.
/// @param modes The modes available for simulation.
/// @param iterations The number of iterations to perform in the search.
///
/// @return The result of the search containing the Pareto fronts.
template <SearchAlgorithm Algorithm, typename State, typename Mode, typename Resources>
auto perform_search(
    const flexman::Manager<State, Mode, Resources> *manager,
    const typename std::vector<Mode> &modes,
    unsigned iterations = 5)
{
    // Check for null pointer in manager.
    if (manager == nullptr) {
        throw std::invalid_argument("manager pointer is null.");
    }

    // Check if iterations is a valid number.
    if (iterations == 0) {
        throw std::invalid_argument("iterations must be greater than 0.");
    }

    // Prepare the result.
    Result<State, Resources> result;

    // We store the pareto front here.
    ParetoFront<State, Resources> pareto_front;

    // A stopwatch, to check runtime.
    timelib::Timer global_timer;

    // Set the timeout.
    if (manager->timeout) {
        global_timer.set_timeout(manager->timeout);
    }

    // Start the timer.
    global_timer.start();

    // Calculate the maximum starting stride factor based on the number of iterations.
    unsigned init_stride;
    if constexpr (Algorithm == SearchAlgorithm::SingleMachine) {
        init_stride = 1U;
    }
    // Start with the highest power of 2.
    else {
        init_stride = 1U << (iterations - 1);
    }

    qinfo(logging::search, "\n");
    qinfo(logging::search, "| Max Iterations | Steps Per Iteration | Time Delta |\n");
    qinfo(logging::search, "|----------------|---------------------|------------|\n");
    for (unsigned steps_per_iteration = init_stride; steps_per_iteration >= 1; steps_per_iteration /= 2) {
        // Calculate the time covered in each iteration.
        const double time_per_iteration = manager->time_delta * static_cast<double>(steps_per_iteration);
        // Determine the maximum number of steps allowed.
        const unsigned max_iterations = static_cast<unsigned>(manager->time_max / time_per_iteration);
        qinfo(logging::search, "| %14u | %19u | %10.6f |\n", max_iterations, steps_per_iteration, time_per_iteration);
    }
    qinfo(logging::search, "\n");

    // Can disable interactive mode.
    bool disable_interactive = false;

    for (unsigned steps_per_iteration = init_stride; steps_per_iteration >= 1; steps_per_iteration /= 2) {
        // Perform a single-pass search.
        pareto_front = flexman::search::perform_search_n_iterations<Algorithm>(
            manager,
            modes,
            steps_per_iteration,
            pareto_front,
            global_timer);

        // Add the pareto front only if it has solutions.
        if (!pareto_front.solutions.empty()) {
            pareto_front.runtime = global_timer.elapsed().count();
            result.pareto_fronts.emplace_back(pareto_front);
        }

        // If we are in interactive mode, pause the search.
        if (!disable_interactive && manager->interactive) {
            // Pause the timer.
            global_timer.pause();

            qwarning(logging::search, "Press 'c' to continue the search, 'r' resume and disable interactive, 'q' to stop it now.\n");

            do {
                char c = flexman::search::wait_for_keypress();
                if (c == 'c') {
                } else if (c == 'r') {
                    disable_interactive = true;
                } else if (c == 'q') {
                    steps_per_iteration = 0;
                } else {
                    continue;
                }
                break;
            } while (true);

            // Resume the timer.
            global_timer.start();
        }

        // Stop if we went into timeout.
        if (global_timer.has_timeout()) {
            qwarning(logging::search, "Stopping at stride factor %3u, because of time-out.\n", steps_per_iteration);
            break;
        }
    }

    return result;
}

} // namespace flexman::search
