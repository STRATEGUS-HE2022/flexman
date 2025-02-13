/// @file manager.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines the `Manager` template class for managing search operations
/// and evaluating solutions.
///
/// @details
/// This file provides the `Manager` template class, which serves as a base
/// for managing search processes. It maintains key parameters such as the
/// initial state, target state, time constraints, and solution evaluation
/// criteria. The class defines virtual methods for:
/// - Updating solutions based on a given mode.
/// - Checking solution completeness.
/// - Measuring the distance between a solution and the target.
/// - Comparing solutions based on strict and probabilistic criteria.
/// - Interpolating states and resources for finer control over transitions.
///
/// The `Manager` class is designed to be extended with specific search
/// strategies, enabling flexible and customizable search management.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <timelib/timespec.hpp>

#include "flexman/core/solution.hpp"

namespace flexman
{

/// @brief Contains the fundamental data structures of the Flexman library.
///
/// @details This namespace defines the core components used throughout the
/// Flexman framework, including modes, solutions, results, and Pareto front
/// representations. These structures serve as the foundation for search,
/// simulation, and optimization processes.
namespace core
{

/// @brief Template class for managing the search.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
template <typename State, typename Mode, class Resources>
class Manager
{
public:
    /// @brief Initial state.
    State initial_state;
    /// @brief Target state.
    State target_state;
    /// @brief Simulation step length (seconds).
    double time_delta{};
    /// @brief Maximal simulation time (seconds).
    double time_max{};
    /// @brief When is a solution considered complete.
    double threshold{};
    /// @brief When should we stop the simulation.
    timelib::timespec_t timeout;
    /// @brief Each step is stopped until the user presses a key.
    bool interactive{};

    /// @brief Default constructor.
    Manager() = default;

    /// @brief Copy constructor.
    ///
    /// @param other the other instance to copy.
    Manager(const Manager &other) = default;

    /// @brief Copy assignment operator.
    ///
    /// @param other the other instance to copy.
    ///
    /// @return a reference to this instance.
    auto operator=(const Manager &other) -> Manager & = default;

    /// @brief Move constructor.
    ///
    /// @param other the other instance to move.
    Manager(Manager &&other) noexcept = default;

    /// @brief Move assignment operator.
    ///
    /// @param other the other instance to move.
    ///
    /// @return a reference to this instance.
    auto operator=(Manager &&other) noexcept -> Manager & = default;

    /// @brief Virtual destructor.
    virtual ~Manager() = default;

    /// @brief Updates the given solution.
    ///
    /// @param solution The solution to be updated.
    /// @param mode The mode used for updating the solution.
    virtual void updated_solution(flexman::core::Solution<State, Resources> &solution, const Mode &mode) const = 0;

    /// @brief Checks if the given solution is complete.
    ///
    /// @param solution The solution to be checked.
    ///
    /// @return True if the solution is complete, false otherwise.
    virtual auto is_complete(const flexman::core::Solution<State, Resources> &solution) const -> bool = 0;

    /// @brief Provides the distance between the given solution and the target.
    ///
    /// @param solution The solution to be measured.
    ///
    /// @return The distance between the solution and the target.
    virtual auto distance(const flexman::core::Solution<State, Resources> &solution) const -> double = 0;

    /// @brief Checks if one solution is better than another.
    ///
    /// @param first The first solution.
    /// @param second The second solution.
    ///
    /// @return True if the first solution is better than the second, false otherwise.
    virtual auto is_strictly_better_than(
        const flexman::core::Solution<State, Resources> &first,
        const flexman::core::Solution<State, Resources> &second) const -> bool = 0;

    /// @brief Checks if one solution is better than another.
    ///
    /// @param first The first solution.
    /// @param second The second solution.
    ///
    /// @return True if the first solution is better than the second, false otherwise.
    virtual auto is_probably_better_than(
        const flexman::core::Solution<State, Resources> &first,
        const flexman::core::Solution<State, Resources> &second) const -> bool = 0;

    /// @brief Compares a solution with another.
    ///
    /// @param first The first solution.
    /// @param second The second solution.
    ///
    /// @return True if the solutions are equal, false otherwise.
    virtual auto is_equal(
        const flexman::core::Solution<State, Resources> &first,
        const flexman::core::Solution<State, Resources> &second) const -> bool = 0;

    /// @brief Interpolates between two resource instances based on a time delta.
    ///
    /// @param r0 The previous set of resources.
    /// @param r1 The new set of resources.
    /// @param rel The relative time factor for interpolation.
    ///
    /// @return Interpolated Resources instance.
    virtual auto interpolate_resources(const Resources &r0, const Resources &r1, double rel) const -> Resources = 0;

    /// @brief Interpolates between two states.
    ///
    /// @param s0 The previous state.
    /// @param s1 The new state.
    /// @param rel The relative time factor for interpolation.
    ///
    /// @return Interpolated Resources instance.
    virtual auto interpolate_state(const State &s0, const State &s1, double rel) const -> State = 0;
};

} // namespace core
} // namespace flexman
