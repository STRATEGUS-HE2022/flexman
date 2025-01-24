/// @file common.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Common search functions.

#pragma once

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <timelib/timer.hpp>

#include "flexman/data_structure/solution.hpp"
#include "flexman/data_structure/manager.hpp"
#include "flexman/data_structure/mode.hpp"
#include "flexman/logging.hpp"

namespace flexman::search
{

/// @brief Defines the available search algorithms for the optimization process.
enum class SearchAlgorithm {
    Exhaustive,   ///< Explores all possible solutions.
    Heuristic,    ///< Uses heuristic methods to guide the search.
    SingleMachine ///< Focuses on a single machine's solution space.
};

/// @brief Represents the switching modes for the optimization process.
enum class SwitchingMode {
    None,       ///< No switching between modes.
    Increasing, ///< Switches between modes in an increasing sequence.
    Free        ///< Allows free switching between modes.
};

/// @brief Logs a set of solutions conditionally based on the specified log level.
///
/// @tparam State The type representing the system's state.
/// @tparam Resources The type representing the system's resources.
///
/// @param logger The logger instance used for logging.
/// @param level The log level to determine whether logging should occur.
/// @param solutions The vector of solutions to log.
template <typename State, typename Resources>
static inline void log_solutions(quire::registry_t::value_t &logger, const quire::log_level level, const std::vector<Solution<State, Resources>> &solutions)
{
    // Check if the logger's level is set to debug or lower.
    if (logger.get_log_level() <= quire::log_level::debug) {
        for (const auto &solution : solutions) {
            qlog(logger, level, "\t%s\n", flexman::to_string(solution).c_str());
        }
    }
}

/// @brief Moves elements from the source vector to the destination vector.
///
/// @tparam T The type of elements stored in the vectors.
///
/// @param source The vector containing elements to move.
/// @param destination The vector to which elements will be moved.
template <typename T>
static inline void move_elements(std::vector<T> &source, std::vector<T> &destination)
{
    // Move the elements to the destination vector.
    destination.insert(destination.end(), std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()));
    // Optionally clear the source if required.
    source.clear();
}

/// @brief Finds the solution closest to zero distance by interpolating between two given solutions.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources used in the system.
///
/// @param manager Pointer to the manager handling the search process.
/// @param previous The previously known solution in the search.
/// @param current The current solution in the search.
///
/// @return The solution closest to zero distance.
template <typename State, typename Mode, class Resources>
Solution<State, Resources> find_solution_closest_to_zero(
    const flexman::Manager<State, Mode, Resources> *manager,
    const Solution<State, Resources> &previous,
    const Solution<State, Resources> &current)
{
    // Get the initial distance to target from the previous solution.
    double distance = std::abs(previous.distance);

    // Calculate step size dynamically based on initial distance and threshold.
    double step_factor = std::max(1.0, distance / manager->threshold);
    double step_size   = manager->time_delta / (10 * step_factor);

    // Variable to store the best complete solution found.
    Solution<State, Resources> solution = previous;

    // Start iterating from low_time to high_time in small steps.
    for (double t = 0, relative; t <= manager->time_delta; t += step_size) {
        // Calculate the relative interpolation factor based on time.
        relative = t / manager->time_delta;

        // Interpolate both the resources and the state between the previous and
        // current solutions.
        solution.resources = manager->interpolate_resources(previous.resources, current.resources, relative);
        solution.state     = manager->interpolate_state(previous.state, current.state, relative);

        // Check if the interpolated solution is complete.
        if (manager->is_complete(solution)) {
            return solution;
        }
    }

    // If no complete solution is found, return the best solution so far (which
    // is the previous solution).
    return current;
}

/// @brief Simulates the mode and produces a new solution.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param search Pointer to the search manager handling the simulation.
/// @param mode The mode being simulated.
/// @param steps The number of steps to simulate.
/// @param solution The initial solution to start the simulation from.
///
/// @return The new solution obtained after the simulation.
template <typename State, typename Mode, class Resources>
inline auto simulate_mode(
    const flexman::Manager<State, Mode, Resources> *search,
    const Mode &mode,
    const unsigned steps,
    Solution<State, Resources> solution)
{
    // Check if search is a valid pointer.
    if (!search) {
        throw std::invalid_argument("search pointer is null");
    }
    // Check if steps is a positive number.
    if (steps == 0) {
        throw std::invalid_argument("steps must be greater than 0");
    }

    Solution<State, Resources> previous;

    // Perform the simulation for the given number of steps, or until the
    // solution is complete.
    for (unsigned i = 0; i < steps; ++i) {
        // Store the previous solution.
        previous = solution;
        // Update the solution.
        search->updated_solution(solution, mode);
        // Add new mode to the sequence.
        flexman::add_mode_execution_to_sequence(mode.id, solution.sequence);
        // If the solution is complete, interpolate to avoid overshoot.
        if (search->is_complete(solution)) {
            return find_solution_closest_to_zero(search, previous, solution);
        }
    }
    // Return the updated solution.
    return solution;
}

/// @brief Extends the given set of partial solutions using the set of modes.
///
/// @tparam SwitchMode The switching mode for simulation.
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the search manager handling the extension process.
/// @param modes The set of modes used to extend the solutions.
/// @param steps_per_iteration The number of steps to simulate per iteration.
/// @param partials The set of partial solutions to extend.
/// @param global_timer The global timer to track the extension process duration.
///
/// @return A new set of extended solutions.
template <SwitchingMode SwitchMode, typename State, typename Mode, class Resources>
auto extend_solutions(
    const flexman::Manager<State, Mode, Resources> *manager,
    const typename std::vector<Mode> &modes,
    const unsigned steps_per_iteration,
    const std::vector<Solution<State, Resources>> &partials,
    const timelib::Timer &global_timer)
{
    // Check if manager is a valid pointer.
    if (!manager) {
        throw std::invalid_argument("manager pointer is null");
    }

    // Check if steps is a positive number.
    if (steps_per_iteration == 0) {
        throw std::invalid_argument("steps_per_iteration must be greater than 0");
    }

    // Check if modes vector is not empty.
    if (modes.empty()) {
        throw std::invalid_argument("modes vector is empty");
    }

    // Prepare a vector for the new solutions.
    std::vector<Solution<State, Resources>> solutions;

    qdebug(logging::common, "[%8u] Before extending set of solutions.\n", partials.size());

    // Iterate over the partial solutions.
    for (const auto &partial : partials) {
        // We freely switch between all available machines.
        if constexpr (SwitchMode == SwitchingMode::Free) {
            // Iterate over the modes.
            for (const auto &mode : modes) {
                // Simulate the given mode and store the new solution.
                solutions.push_back(simulate_mode(manager, mode, steps_per_iteration, partial));
            }
        }
        // We switch to only subsequent machines.
        else if constexpr (SwitchMode == SwitchingMode::Increasing) {
            // Iterate over the modes.
            for (mode_id_t mode = partial.sequence.back(); mode < modes.size(); ++mode) {
                // Simulate the given mode and store the new solution.
                solutions.push_back(simulate_mode(manager, modes[mode], steps_per_iteration, partial));
            }
        }
        // Simple case without any switching.
        else {
            // Simulate the given mode and store the new solution.
            solutions.push_back(simulate_mode(manager, modes[partial.sequence.back().mode], steps_per_iteration, partial));
        }
        // Check if the timer has expired.
        if (global_timer.has_timeout()) {
            qwarning(logging::common, "Timer expired while extending solutions.\n");
            break;
        }
    }

    qdebug(logging::common, "[%8u] After extending set of solutions.\n", solutions.size());

    // Return the new set of solutions.
    return solutions;
}

/// @brief Removes solutions that are dominated by any solution in the given set.
///
/// @tparam Algorithm The search algorithm used to evaluate dominance.
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the search manager handling the process.
/// @param solutions The set of solutions to filter.
/// @param solutions_to_check_against The set of solutions to check for dominance.
template <SearchAlgorithm Algorithm, typename State, typename Mode, class Resources>
void remove_dominated_solutions(
    const flexman::Manager<State, Mode, Resources> *manager,
    std::vector<Solution<State, Resources>> &solutions,
    const std::vector<Solution<State, Resources>> &solutions_to_check_against)
{
    // Check if manager is a valid pointer.
    if (!manager) {
        throw std::invalid_argument("manager pointer is null");
    }

    if (&solutions == &solutions_to_check_against) {
        throw std::invalid_argument("solutions and solutions_to_check_against must not be the same.");
    }

    qdebug(logging::common, "[%8u] Before removing dominated solutions.\n", solutions.size());

    // Check if solutions_to_check_against vecto is empty.
    if (solutions_to_check_against.empty()) {
        qdebug(logging::common, "[%8u] After removing dominated solutions (SAME).\n", solutions.size());
        return;
    }

    // Erase the solutions that are dominated.
    std::erase_if(solutions, [&](const Solution<State, Resources> &solution) {
        return std::any_of(
            solutions_to_check_against.begin(),
            solutions_to_check_against.end(),
            [&](const auto &other_solution) {
                if constexpr (Algorithm == SearchAlgorithm::Heuristic) {
                    return manager->is_probably_better_than(other_solution, solution);
                } else {
                    return manager->is_strictly_better_than(other_solution, solution);
                }
            });
    });

    qdebug(logging::common, "[%8u] After removing dominated solutions.\n", solutions.size());
}

/// @brief Removes dominated solutions from a vector.
///
/// @tparam Algorithm The search algorithm used to evaluate dominance.
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the search manager handling the process.
/// @param solutions The vector of solutions to filter, also used as the reference for dominance checks.
template <SearchAlgorithm Algorithm, typename State, typename Mode, typename Resources>
void remove_dominated_solutions(
    const flexman::Manager<State, Mode, Resources> *manager,
    std::vector<Solution<State, Resources>> &solutions)
{
    // Check if manager is a valid pointer.
    if (!manager) {
        throw std::invalid_argument("manager pointer is null");
    }

    qdebug(logging::common, "[%8u] Before removing dominated solutions.\n", solutions.size());

    // Check if solutions_to_check_against vecto is empty.
    if (solutions.empty()) {
        qdebug(logging::common, "[%8u] After removing dominated solutions (SAME).\n", solutions.size());
        return;
    }

    // Vector to store indices of solutions to keep.
    std::vector<std::size_t> solutions_to_keep_idx;
    solutions_to_keep_idx.reserve(solutions.size());

    // Identify solutions to keep.
    for (std::size_t i = 0; i < solutions.size(); ++i) {
        const auto &solution = solutions[i];
        bool is_dominated    = false;
        for (const auto &other_solution : solutions) {
            // Skip comparing a solution with itself.
            if (&solution == &other_solution) {
                continue;
            }
            // Check dominance.
            if constexpr (Algorithm == SearchAlgorithm::Heuristic) {
                if (manager->is_probably_better_than(other_solution, solution)) {
                    is_dominated = true;
                    break;
                }
            } else {
                if (manager->is_strictly_better_than(other_solution, solution)) {
                    is_dominated = true;
                    break;
                }
            }
        }
        if (!is_dominated) {
            solutions_to_keep_idx.push_back(i);
        }
    }

    // Rebuild the solutions vector with only the kept solutions.
    std::vector<Solution<State, Resources>> filtered_solutions;
    filtered_solutions.reserve(solutions_to_keep_idx.size());

    for (std::size_t idx : solutions_to_keep_idx) {
        filtered_solutions.push_back(std::move(solutions[idx]));
    }

    // Swap filtered solutions back into the original vector.
    solutions.swap(filtered_solutions);
}

/// @brief Removes duplicate solutions from the given set of solutions.
///
/// @tparam State The type representing the state.
/// @tparam Resources The type representing the resources.
///
/// @param solutions The vector of solutions to filter for duplicates.
template <typename State, class Resources>
void remove_duplicate_solutions(std::vector<Solution<State, Resources>> &solutions)
{
    qdebug(logging::common, "[%8u] Before removing duplicate solutions.\n", solutions.size());

    // First, sort the solutions to bring duplicates together.
    std::sort(solutions.begin(), solutions.end());
    // Then, erase the duplicates from the vector.
    solutions.erase(std::unique(solutions.begin(), solutions.end()), solutions.end());

    qdebug(logging::common, "[%8u] After removing duplicate solutions.\n", solutions.size());
}

/// @brief Splits the given set of solutions into complete and partial solutions.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param manager Pointer to the search manager handling the process.
/// @param solutions The vector of solutions to split.
/// @param complete The vector to store complete solutions.
/// @param partial The vector to store partial solutions.
template <typename State, typename Mode, class Resources>
void split_complete_partial(
    const flexman::Manager<State, Mode, Resources> *manager,
    std::vector<Solution<State, Resources>> &solutions,
    std::vector<Solution<State, Resources>> &complete,
    std::vector<Solution<State, Resources>> &partial)
{
    // Check if manager is a valid pointer.
    if (!manager) {
        throw std::invalid_argument("manager pointer is null");
    }

    // Check if solutions vector is not empty.
    if (!solutions.empty()) {
        qdebug(logging::common, "[%8u] Before splitting among complete and partial solutions.\n", solutions.size());

        // Partition the solutions into complete and partial in-place.
        auto it = std::partition(
            solutions.begin(), solutions.end(),
            [&manager](const auto &solution) -> bool {
                return manager->is_complete(solution);
            });

        // Move the complete solutions to the complete vector.
        std::move(solutions.begin(), it, std::back_inserter(complete));

        // Move the partial solutions to the partial vector.
        std::move(it, solutions.end(), std::back_inserter(partial));

        // Clear the solutions vector after moving.
        solutions.clear();

        // We split between complete solutions and partial ones.
        qdebug(logging::common, "[%8u] After among complete and partial solutions [complete: %8u, partial: %8u]\n",
               solutions.size(), complete.size(), partial.size());
    }
}

/// @brief Waits for a key press and returns the pressed character.
/// @return The character of the key pressed.
char wait_for_keypress()
{
#ifdef _WIN32
    // Windows implementation using _getch()
    return _getch(); // Return the character directly
#else
    // Linux/Unix implementation using termios
    struct termios oldt, newt;
    char ch;

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echoing
    newt.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Read a single character
    read(STDIN_FILENO, &ch, 1);

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch; // Return the character
#endif
}

} // namespace flexman::search