/// @file search.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines discrete and continuous search managers for manufacturing.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "builder.hpp"

namespace manufacturing
{

/// @brief Search manager for discrete manufacturing systems.
class discrete_search_t : public flexman::core::Manager<state_t, discrete_mode_t, resources_t>
{
public:
    using solution_t = flexman::core::Solution<state_t, resources_t>;

    parameters_t params;     ///< Parameters for the manufacturing system.
    state_t target_state;    ///< Target state for completion checking
    double threshold  = 0.1; ///< Distance threshold for completion
    double time_delta = 1.0; ///< Time step for discrete updates

    discrete_search_t() = default;

    void updated_solution(solution_t &solution, const discrete_mode_t &mode) const override
    {
        // Update the state using the discrete system dynamics
        solution.state = fsmlib::multiply(mode.system.A, solution.state) + fsmlib::multiply(mode.system.B, mode.input);

        // Update distance to target
        solution.distance = this->distance(solution);

        // Calculate resource consumption for this step
        resources_t step_resources;
        step_resources.time           = time_delta;
        step_resources.energy         = mode.operation_cost.energy * time_delta;
        step_resources.energy_cost    = mode.operation_cost.energy_cost * time_delta;
        step_resources.material_cost  = mode.operation_cost.material_cost * time_delta;
        step_resources.labor_cost     = mode.operation_cost.labor_cost * time_delta;
        step_resources.equipment_wear = mode.operation_cost.equipment_wear * time_delta;
        step_resources.quality_loss   = mode.operation_cost.quality_loss * time_delta;

        // Accumulate resources
        solution.resources += step_resources;
    }

    bool is_complete(const solution_t &solution) const override
    {
        return solution.distance < threshold;
    }

    double distance(const solution_t &solution) const override
    {
        return calculate_distance_to_target(solution.state);
    }

    bool is_strictly_better_than(const solution_t &lhs, const solution_t &rhs) const override
    {
        // Multi-objective comparison: time and total cost
        double lhs_cost = lhs.resources.total_cost();
        double rhs_cost = rhs.resources.total_cost();

        if (lhs.resources.time < rhs.resources.time && lhs_cost <= rhs_cost) {
            return true;
        }
        if (lhs_cost < rhs_cost && lhs.resources.time <= rhs.resources.time) {
            return true;
        }
        return false;
    }

    bool is_probably_better_than(const solution_t &lhs, const solution_t &rhs) const override
    {
        // Weighted combination of time and cost
        double lhs_objective = lhs.resources.time + lhs.resources.total_cost();
        double rhs_objective = rhs.resources.time + rhs.resources.total_cost();
        return lhs_objective < rhs_objective;
    }

    bool is_equal(const solution_t &lhs, const solution_t &rhs) const override
    {
        return lhs.resources == rhs.resources;
    }

    resources_t interpolate_resources(const resources_t &r0, const resources_t &r1, double rel) const override
    {
        resources_t result;
        result.time           = r0.time + rel * (r1.time - r0.time);
        result.energy         = r0.energy + rel * (r1.energy - r0.energy);
        result.energy_cost    = r0.energy_cost + rel * (r1.energy_cost - r0.energy_cost);
        result.material_cost  = r0.material_cost + rel * (r1.material_cost - r0.material_cost);
        result.labor_cost     = r0.labor_cost + rel * (r1.labor_cost - r0.labor_cost);
        result.equipment_wear = r0.equipment_wear + rel * (r1.equipment_wear - r0.equipment_wear);
        result.quality_loss   = r0.quality_loss + rel * (r1.quality_loss - r0.quality_loss);
        return result;
    }

    state_t interpolate_state(const state_t &s0, const state_t &s1, double rel) const override
    {
        state_t result;
        for (std::size_t i = 0; i < result.size(); ++i) {
            result[i] = s0[i] + rel * (s1[i] - s0[i]);
        }
        return result;
    }

    bool can_switch(const discrete_mode_t &, const discrete_mode_t &) const override
    {
        // All machines can switch to any other machine (no constraints for now)
        return true;
    }

    resources_t get_switch_cost(const state_t &, const discrete_mode_t &from, const discrete_mode_t &to) const override
    {
        // Simple switching cost based on machine type change
        resources_t switch_cost;
        if (from.id != to.id) {
            switch_cost.time       = 5.0;  // 5 seconds setup time
            switch_cost.labor_cost = 10.0; // Setup cost
        }
        return switch_cost;
    }

private:
    double calculate_distance_to_target(const state_t &state) const
    {
        double distance = 0.0;
        for (std::size_t i = 0; i < state.size() && i < target_state.size(); ++i) {
            double diff = state[i] - target_state[i];
            distance += diff * diff;
        }
        return std::sqrt(distance);
    }
};

/// @brief Search manager for continuous manufacturing systems.
class continuous_search_t : public flexman::core::Manager<state_t, continuous_mode_t, resources_t>
{
public:
    using solution_t = flexman::core::Solution<state_t, resources_t>;

    parameters_t params;     ///< Parameters for the manufacturing system.
    state_t target_state;    ///< Target state for completion checking
    double threshold  = 0.1; ///< Distance threshold for completion
    double time_delta = 0.1; ///< Time step for continuous integration

    continuous_search_t() = default;

    void updated_solution(solution_t &solution, const continuous_mode_t &mode) const override
    {
        // For continuous systems, we integrate the differential equation
        // x_dot = A*x + B*u
        state_t state_dot = fsmlib::multiply(mode.system.A, solution.state) + fsmlib::multiply(mode.system.B, mode.input);

        // Simple Euler integration
        for (std::size_t i = 0; i < solution.state.size(); ++i) {
            solution.state[i] += state_dot[i] * time_delta;
        }

        // Update distance to target
        solution.distance = this->distance(solution);

        // Calculate resource consumption for this step
        resources_t step_resources;
        step_resources.time           = time_delta;
        step_resources.energy         = mode.operation_cost.energy * time_delta;
        step_resources.energy_cost    = mode.operation_cost.energy_cost * time_delta;
        step_resources.material_cost  = mode.operation_cost.material_cost * time_delta;
        step_resources.labor_cost     = mode.operation_cost.labor_cost * time_delta;
        step_resources.equipment_wear = mode.operation_cost.equipment_wear * time_delta;
        step_resources.quality_loss   = mode.operation_cost.quality_loss * time_delta;

        // Accumulate resources
        solution.resources += step_resources;
    }

    bool is_complete(const solution_t &solution) const override
    {
        return solution.distance < threshold;
    }

    double distance(const solution_t &solution) const override
    {
        return calculate_distance_to_target(solution.state);
    }

    bool is_strictly_better_than(const solution_t &lhs, const solution_t &rhs) const override
    {
        // Multi-objective comparison: time and total cost
        double lhs_cost = lhs.resources.total_cost();
        double rhs_cost = rhs.resources.total_cost();

        if (lhs.resources.time < rhs.resources.time && lhs_cost <= rhs_cost) {
            return true;
        }
        if (lhs_cost < rhs_cost && lhs.resources.time <= rhs.resources.time) {
            return true;
        }
        return false;
    }

    bool is_probably_better_than(const solution_t &lhs, const solution_t &rhs) const override
    {
        // Weighted combination of time and cost
        double lhs_objective = lhs.resources.time + lhs.resources.total_cost();
        double rhs_objective = rhs.resources.time + rhs.resources.total_cost();
        return lhs_objective < rhs_objective;
    }

    bool is_equal(const solution_t &lhs, const solution_t &rhs) const override
    {
        return lhs.resources == rhs.resources;
    }

    resources_t interpolate_resources(const resources_t &r0, const resources_t &r1, double rel) const override
    {
        resources_t result;
        result.time           = r0.time + rel * (r1.time - r0.time);
        result.energy         = r0.energy + rel * (r1.energy - r0.energy);
        result.energy_cost    = r0.energy_cost + rel * (r1.energy_cost - r0.energy_cost);
        result.material_cost  = r0.material_cost + rel * (r1.material_cost - r0.material_cost);
        result.labor_cost     = r0.labor_cost + rel * (r1.labor_cost - r0.labor_cost);
        result.equipment_wear = r0.equipment_wear + rel * (r1.equipment_wear - r0.equipment_wear);
        result.quality_loss   = r0.quality_loss + rel * (r1.quality_loss - r0.quality_loss);
        return result;
    }

    state_t interpolate_state(const state_t &s0, const state_t &s1, double rel) const override
    {
        state_t result;
        for (std::size_t i = 0; i < result.size(); ++i) {
            result[i] = s0[i] + rel * (s1[i] - s0[i]);
        }
        return result;
    }

    bool can_switch(const continuous_mode_t &, const continuous_mode_t &) const override
    {
        // All machines can switch to any other machine (no constraints for now)
        return true;
    }

    resources_t get_switch_cost(const state_t &, const continuous_mode_t &from, const continuous_mode_t &to) const override
    {
        // Simple switching cost based on machine type change
        resources_t switch_cost;
        if (from.id != to.id) {
            switch_cost.time       = 0.5; // 0.5 seconds setup time for continuous
            switch_cost.labor_cost = 5.0; // Setup cost
        }
        return switch_cost;
    }

private:
    double calculate_distance_to_target(const state_t &state) const
    {
        double distance = 0.0;
        for (std::size_t i = 0; i < state.size() && i < target_state.size(); ++i) {
            double diff = state[i] - target_state[i];
            distance += diff * diff;
        }
        return std::sqrt(distance);
    }
};

} // namespace manufacturing

namespace json
{

/// @brief JSON serialization for discrete_search_t.
inline json::jnode_t &operator<<(json::jnode_t &lhs, const manufacturing::discrete_search_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["initial_state"] << rhs.initial_state;
    lhs["target_state"] << rhs.target_state;
    return lhs;
}

/// @brief JSON deserialization for discrete_search_t.
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, manufacturing::discrete_search_t &rhs)
{
    lhs["initial_state"] >> rhs.initial_state;
    lhs["target_state"] >> rhs.target_state;
    return lhs;
}

/// @brief JSON serialization for continuous_search_t.
inline json::jnode_t &operator<<(json::jnode_t &lhs, const manufacturing::continuous_search_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["initial_state"] << rhs.initial_state;
    lhs["target_state"] << rhs.target_state;
    return lhs;
}

/// @brief JSON deserialization for continuous_search_t.
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, manufacturing::continuous_search_t &rhs)
{
    lhs["initial_state"] >> rhs.initial_state;
    lhs["target_state"] >> rhs.target_state;
    return lhs;
}

} // namespace json
