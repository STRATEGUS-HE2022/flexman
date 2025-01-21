/// @file model.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include "builder.hpp"

#include <chainsaw/stepper/stepper_rk4.hpp>
#include <chainsaw/detail/observer.hpp>
#include <chainsaw/solver.hpp>

namespace tapping
{

class discrete_search_t : public flexman::Manager<state_t, discrete_mode_t, resources_t> {
public:
    discrete_search_t() = default;

    void updated_solution(solution_t &solution, const discrete_mode_t &mode) const override
    {
        // Update the state.
        solution.state = fsmlib::multiply(mode.system.A, solution.state) + fsmlib::multiply(mode.system.B, mode.input);
        // Update the distance.
        solution.distance = this->distance(solution);
        // Update energy.
        solution.resources.energy += solution.state[1] * mode.input[0] * time_delta;
        // Update time.
        solution.resources.time += time_delta;
    }

    double distance(const solution_t &solution) const override
    {
        return target_state[2] - solution.state[2];
    }

    bool is_complete(const solution_t &solution) const override
    {
        return this->distance(solution) < threshold;
    }

    bool is_strictly_better_than(const solution_t &x, const solution_t &y) const override
    {
        if (x.sequence == y.sequence) {
            return false;
        }
        return this->is_complete(x) && (x.resources <= y.resources) && (x.resources != y.resources);
    }

    bool is_probably_better_than(const solution_t &x, const solution_t &y) const override
    {
        if (x.sequence == y.sequence) {
            return false;
        }
        const auto xd = this->distance(x);
        const auto yd = this->distance(y);
        if ((xd <= yd) && (x.resources <= y.resources)) {
            return (xd < yd) || (x.resources < y.resources);
        }
        return false;
    }

    bool is_equal(const solution_t &x, const solution_t &y) const override
    {
        return (x.sequence == y.sequence) || (x.resources == y.resources);
    }

    resources_t interpolate_resources(const resources_t &r0, const resources_t &r1, double rel) const override
    {
        // Linear interpolation.
        return resources_t{
            .energy = r0.energy + rel * (r1.energy - r0.energy),
            .time   = r0.time + rel * (r1.time - r0.time),
        };
    }

    state_t interpolate_state(const state_t &s0, const state_t &s1, double rel) const override
    {
        state_t interpolated_state = s0; // Start by copying s0
        for (size_t i = 0; i < s0.size(); ++i) {
            interpolated_state[i] = s0[i] + rel * (s1[i] - s0[i]);
        }
        return interpolated_state;
    }
};

class continuous_search_t : public flexman::Manager<state_t, continous_mode_t, resources_t> {
public:
    continuous_search_t() = default;

    void updated_solution(solution_t &solution, const continous_mode_t &mode) const override
    {
        // Update the state.
        chainsaw::stepper_rk4<state_t, double> solver;
        chainsaw::detail::Observer<state_t, double> observer;
        // Compute the step_size.
        double step_size = time_delta / 100;
        // Perform integration.
        chainsaw::integrate_fixed(
            solver,
            observer,
            [&](const state_t &x, state_t &dxdt, double) {
                // Advance system state.
                dxdt = fsmlib::multiply(mode.system.A, x) + fsmlib::multiply(mode.system.B, mode.input);
            },
            solution.state,
            solution.resources.time, solution.resources.time + time_delta,
            step_size,
            [&](const state_t &x) {
                return (target_state[2] - x[2]) < threshold;
            });
        // Update the distance.
        solution.distance = this->distance(solution);
        // Update energy.
        solution.resources.energy += solution.state[1] * mode.input[0] * time_delta;
        // Update time.
        solution.resources.time += time_delta;
    }

    double distance(const solution_t &solution) const override
    {
        return target_state[2] - solution.state[2];
    }

    bool is_complete(const solution_t &solution) const override
    {
        return this->distance(solution) < threshold;
    }

    bool is_strictly_better_than(const solution_t &x, const solution_t &y) const override
    {
        if (x.sequence == y.sequence) {
            return false;
        }
        return this->is_complete(x) && (x.resources <= y.resources) && (x.resources != y.resources);
    }

    bool is_probably_better_than(const solution_t &x, const solution_t &y) const override
    {
        if (x.sequence == y.sequence) {
            return false;
        }
        const auto xd = this->distance(x);
        const auto yd = this->distance(y);
        if ((xd <= yd) && (x.resources <= y.resources)) {
            return (xd < yd) || (x.resources < y.resources);
        }
        return false;
    }

    bool is_equal(const solution_t &x, const solution_t &y) const override
    {
        return (x.sequence == y.sequence) || (x.resources == y.resources);
    }

    resources_t interpolate_resources(const resources_t &r0, const resources_t &r1, double rel) const override
    {
        // Linear interpolation.
        return resources_t{
            .energy = r0.energy + rel * (r1.energy - r0.energy),
            .time   = r0.time + rel * (r1.time - r0.time),
        };
    }

    state_t interpolate_state(const state_t &s0, const state_t &s1, double rel) const override
    {
        state_t interpolated_state = s0; // Start by copying s0
        for (size_t i = 0; i < s0.size(); ++i) {
            interpolated_state[i] = s0[i] + rel * (s1[i] - s0[i]);
        }
        return interpolated_state;
    }
};

} // namespace tapping

namespace json
{

inline json::jnode_t &operator<<(json::jnode_t &lhs, const timelib::timespec_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["tv_sec"] << rhs.tv_sec;
    lhs["tv_nsec"] << rhs.tv_nsec;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, timelib::timespec_t &rhs)
{
    lhs["tv_sec"] >> rhs.tv_sec;
    lhs["tv_nsec"] >> rhs.tv_nsec;
    return lhs;
}

inline json::jnode_t &operator<<(json::jnode_t &lhs, const tapping::discrete_search_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["initial_state"] << rhs.initial_state;
    lhs["target_state"] << rhs.target_state;
    lhs["time_delta"] << rhs.time_delta;
    lhs["time_max"] << rhs.time_max;
    lhs["threshold"] << rhs.threshold;
    lhs["timeout"] << rhs.timeout;
    lhs["interactive"] << rhs.interactive;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, tapping::discrete_search_t &rhs)
{
    lhs["initial_state"] >> rhs.initial_state;
    lhs["target_state"] >> rhs.target_state;
    lhs["time_delta"] >> rhs.time_delta;
    lhs["time_max"] >> rhs.time_max;
    lhs["threshold"] >> rhs.threshold;
    lhs["timeout"] >> rhs.timeout;
    lhs["interactive"] >> rhs.interactive;
    return lhs;
}

inline json::jnode_t &operator<<(json::jnode_t &lhs, const tapping::continuous_search_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["initial_state"] << rhs.initial_state;
    lhs["target_state"] << rhs.target_state;
    lhs["time_delta"] << rhs.time_delta;
    lhs["time_max"] << rhs.time_max;
    lhs["threshold"] << rhs.threshold;
    lhs["timeout"] << rhs.timeout;
    lhs["interactive"] << rhs.interactive;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, tapping::continuous_search_t &rhs)
{
    lhs["initial_state"] >> rhs.initial_state;
    lhs["target_state"] >> rhs.target_state;
    lhs["time_delta"] >> rhs.time_delta;
    lhs["time_max"] >> rhs.time_max;
    lhs["threshold"] >> rhs.threshold;
    lhs["timeout"] >> rhs.timeout;
    lhs["interactive"] >> rhs.interactive;
    return lhs;
}

} // namespace json
