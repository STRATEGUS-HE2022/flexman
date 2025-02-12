/// @file manager.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Main PSO search functions.

#pragma once

#include <timelib/timespec.hpp>

namespace flexman
{

/// @brief Template class for managing the search.
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
template <typename State, typename Mode, class Resources>
class Manager
{
public:
    State initial_state;         ///< Initial state.
    State target_state;          ///< Target state.
    double time_delta;           ///< Simulation step length (seconds).
    double time_max;             ///< Maximal simulation time (seconds).
    double threshold;            ///< When is a solution considered complete.
    timelib::timespec_t timeout; ///< When should we stop the simulation.
    bool interactive;            ///< Each step is stopped until the user presses a key.

    /// @brief Default constructor.
    Manager() = default;

    /// @brief Virtual destructor.
    virtual ~Manager() = default;

    /// @brief Updates the given solution.
    /// @param solution The solution to be updated.
    /// @param mode The mode used for updating the solution.
    virtual void updated_solution(Solution<State, Resources> &solution, const Mode &mode) const = 0;

    /// @brief Checks if the given solution is complete.
    /// @param solution The solution to be checked.
    /// @return True if the solution is complete, false otherwise.
    virtual bool is_complete(const Solution<State, Resources> &solution) const = 0;

    /// @brief Provides the distance between the given solution and the target.
    /// @param solution The solution to be measured.
    /// @return The distance between the solution and the target.
    virtual double distance(const Solution<State, Resources> &solution) const = 0;

    /// @brief Checks if one solution is better than another.
    /// @param first The first solution.
    /// @param second The second solution.
    /// @return True if the first solution is better than the second, false otherwise.
    virtual bool is_strictly_better_than(
        const Solution<State, Resources> &first,
        const Solution<State, Resources> &second) const = 0;

    /// @brief Checks if one solution is better than another.
    /// @param first The first solution.
    /// @param second The second solution.
    /// @return True if the first solution is better than the second, false otherwise.
    virtual bool is_probably_better_than(
        const Solution<State, Resources> &first,
        const Solution<State, Resources> &second) const = 0;

    /// @brief Compares a solution with another.
    /// @param first The first solution.
    /// @param second The second solution.
    /// @return True if the solutions are equal, false otherwise.
    virtual bool is_equal(const Solution<State, Resources> &first, const Solution<State, Resources> &second) const = 0;

    /// @brief Interpolates between two resource instances based on a time delta.
    /// @param r0 The previous set of resources.
    /// @param r1 The new set of resources.
    /// @param rel The relative time factor for interpolation.
    /// @return Interpolated Resources instance.
    virtual Resources interpolate_resources(const Resources &r0, const Resources &r1, double rel) const = 0;

    /// @brief Interpolates between two states.
    /// @param s0 The previous state.
    /// @param s1 The new state.
    /// @param rel The relative time factor for interpolation.
    /// @return Interpolated Resources instance.
    virtual State interpolate_state(const State &s0, const State &s1, double rel) const = 0;
};

} // namespace flexman
