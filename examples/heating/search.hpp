/// @file model.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines discrete and continuous search managers for Flexman.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "builder.hpp"

#include <numint/detail/observer.hpp>
#include <numint/solver.hpp>
#include <numint/stepper/stepper_rk4.hpp>

namespace heating
{

/// @brief Compute leak and fan power from heater power u.
struct leak_result_t {
    double G_leak;
    double P_fan;
};

inline leak_result_t leak_and_fan(parameters_t p, double u) noexcept
{
    /*
    ------------------------------------------------------------------------------
    Make high power cost more electricity (fan power tied to leak)
    ------------------------------------------------------------------------------

    Idea
    ----
    We keep the 2-state thermal model and let the environment leak to the room
    grow with the commanded heater power u [W]. Physically, more power often
    means more airflow (fan/jet/steam) and thus larger convective losses. We
    also account for the fan’s *electrical* draw as a function of that leak.
    This creates diminishing returns: blasting heat raises losses and fan cost,
    so “always max power” is no longer automatically optimal.

    Leak conductance model
    ----------------------
    Let:
    u         = heater power command [W]
    G_leak(u) = environment->room leak conductance [W/°C]

    We model:
    G_leak(u) = clamp( G0 + k_leak * u, Gmin, Gmax )

    Where:
    G0        baseline leak at zero power [W/°C]
    k_leak    leak gain per watt of heater power [(W/°C)/W]
    Gmin/max  safety bounds (avoid negative or absurd values)

    Fan electrical power model
    --------------------------
    Fan/flow power scales ~ cubic with airflow; we use a cubic in (G_leak/G0):

    P_fan(G_leak) = Pf0 + Pf_cubic * ( G_leak / G0 )^3          [W]

    Where:
    Pf0       idle fan draw at baseline leak [W]
    Pf_cubic  scale factor for the cubic term [W]
    (Use clamp to keep G_leak/G0 >= 0; require G0 > 0.)

    Thermal state equations (continuous-time)
    -----------------------------------------
    States: x = [ T,  T_env ]^T  [°C]
    Input:  u = heater power      [W]
    Params: G = h*area            [W/°C]
            Cx = mass*Cp0         [J/°C]
            Ce = env_mass*Cp_env  [J/°C]
            η  = efficiency       [–]
            T_inf = room temp     [°C] (constant)

    Cx * dT/dt     = -G * (T - T_env) + η * u
    Ce * dT_env/dt =  G * (T - T_env) - G_leak(u) * (T_env - T_inf)

    Notes:
    - Because G_leak depends on u, the env equation has a u * T_env product.
    The model is *bilinear/time-varying*, not a single fixed (A,B).
    - Implementation: at each integration step, recompute the effective A
    using the current G_leak(u). B is unchanged (u enters the workpiece).

    Energy accounting (per step Δt)
    -------------------------------
    Electrical energy consumed:
    E_elec += ( u + P_fan(G_leak(u)) ) * Δt

    Thermal energy delivered to the thermal network:
    E_heat += ( η * u ) * Δt

    Optional: energy leaked to room (diagnostic):
    E_leak += ( G_leak(u) * (T_env - T_inf) ) * Δt

    Steady-state intuition (after T ≈ T_env)
    ----------------------------------------
    Approximate steady rise above room (°C) under constant u:

    ΔT_inf(u) ≈ ( η * u ) / ( G + G_leak(u) )

    Using G_leak(u) = G0 + k_leak*u:
    ΔT_inf(u) ≈ ( η * u ) / ( G + G0 + k_leak*u )

    This *saturates* as u grows (denominator grows with u), so pushing more
    power yields diminishing temperature returns while fan/electrical cost rises.

    Typical tuning (example)
    ------------------------
    G0 = 1.0 W/°C
    k_leak = 0.004 (W/°C)/W   // +2.4 W/°C at 600 W
    Gmin = 0.5, Gmax = 6.0 W/°C
    Pf0 = 8 W
    Pf_cubic = 8 W            // fan power doubles roughly when G_leak ≈ 2*G0

    Practical cautions
    ------------------
    - Recompute A each step after evaluating G_leak(u). If your code assumes a
    fixed A over the whole horizon, switch to per-step A or use a “hybrid”
    approach (tie discrete power modes to fixed leak values).
    - Keep units consistent. Enforce G0 > 0 to avoid divide-by-zero in (G_leak/G0)^3.
    - Log effective values to make behavior transparent:
        pr_debug("u=%.1f W, G_leak=%.2f W/C, P_fan=%.1f W", u, G_leak, P_fan);

    Result
    ------
    Higher power now increases both convective losses and auxiliary fan draw.
    Your optimizer will naturally trade “time vs. electricity” instead of
    always picking the strongest setting.
    */
    // Tunables: pick these once to shape your Pareto
    const double G0     = p.G_leak0;      // [W/°C]
    const double kL     = p.k_leak_per_w; // [W/°C per W]
    const double Gmin   = p.G_leak_min;
    const double Gmax   = p.G_leak_max;
    const double Pf0    = p.fan_P0;      // [W] idle fan draw at G0
    const double Pf_cub = p.fan_P_cubic; // [W] scale of cubic term

    // G_leak(u)
    const double G_eff = std::clamp(G0 + kL * u, Gmin, Gmax);

    // Fan power P_fan ≈ Pf0 + Pf_cub * (G_eff/G0)^3
    const double ratio = std::max(G_eff / std::max(G0, 1e-9), 0.0);
    const double P_fan = Pf0 + Pf_cub * ratio * ratio * ratio;

    return {G_eff, P_fan};
}

class discrete_search_t : public flexman::core::Manager<state_t, discrete_mode_t, resources_t>
{
public:
    parameters_t params; ///< Parameters for the heating system.

    discrete_search_t() = default;

    void updated_solution(solution_t &solution, const discrete_mode_t &mode) const override
    {
        // Update the state.
        solution.state           = fsmlib::multiply(mode.system.A, solution.state) + fsmlib::multiply(mode.system.B, mode.input);
        // Update distance
        solution.distance        = this->distance(solution);
        // Update energy budgets
        const double u           = mode.input[0]; // power [W]
        // Electrical energy consumed [J]
        const auto lr            = leak_and_fan(mode.parameters, u);
        const double energy_elec = (u + lr.P_fan) * time_delta; //u * time_delta;
        // Heat energy produced [J]
        const double energy_heat = (params.eta * u) * time_delta;
        solution.resources.energy += energy_elec + energy_heat;
        // Update time
        solution.resources.time += time_delta;
#if 0
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "[ " << std::setw(6) << std::right << solution.resources.time << " ] ";
        std::cout << "[ " << std::setw(2) << std::right << static_cast<unsigned>(mode.id) << " ] ";
        std::cout << std::setw(3) << std::left << mode.input[0] << " -> [";
        std::cout << std::setw(4) << std::right << solution.state[0] << ", ";
        std::cout << std::setw(4) << std::right << solution.state[1] << "] ";
        std::cout << std::setw(4) << std::right << solution.distance << "\n";
#endif
    }

    double distance(const solution_t &solution) const override { return target_state[0] - solution.state[0]; }

    bool is_complete(const solution_t &solution) const override { return this->distance(solution) < threshold; }

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

    bool can_switch([[maybe_unused]] const discrete_mode_t &from, [[maybe_unused]] const discrete_mode_t &to) const override
    {
        // For now, allow any switch. This can be customized later.
        return true;
    }

    resources_t get_switch_cost([[maybe_unused]] const state_t &current_state, [[maybe_unused]] const discrete_mode_t &from, [[maybe_unused]] const discrete_mode_t &to) const override
    {
        // For now, return zero resources. This can be customized later.
        return resources_t{0.0, 0.0};
    }
};

class continuous_search_t : public flexman::core::Manager<state_t, continuous_mode_t, resources_t>
{
public:
    parameters_t params; ///< Parameters for the heating system.

    continuous_search_t() = default;

    void updated_solution(solution_t &solution, const continuous_mode_t &mode) const override
    {
        // Update the state.
        numint::stepper_rk4<state_t, double> solver;
        numint::detail::Observer<state_t, double> observer;
        // Compute the step_size.
        double step_size = time_delta / 100;
        // Perform integration.
        numint::integrate_fixed(
            solver, observer,
            [&](const state_t &x, state_t &dxdt, double) {
                // Advance system state.
                dxdt = fsmlib::multiply(mode.system.A, x) + fsmlib::multiply(mode.system.B, mode.input);
            },
            solution.state, solution.resources.time, solution.resources.time + time_delta, step_size,
            [&](const state_t &x) { return (target_state[0] - x[0]) < threshold; });
        // Update the distance.
        solution.distance        = this->distance(solution);
        // Update energy budgets.
        const double u           = mode.input[0]; // power [W]
        // Electrical energy consumed [J]
        const double energy_elec = u * time_delta;
        // Heat energy produced [J]
        const double energy_heat = (params.eta * u) * time_delta;
        solution.resources.energy += energy_elec + energy_heat;
        // Update time.
        solution.resources.time += time_delta;
    }

    double distance(const solution_t &solution) const override { return target_state[0] - solution.state[0]; }

    bool is_complete(const solution_t &solution) const override { return this->distance(solution) < threshold; }

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

    bool can_switch([[maybe_unused]] const continuous_mode_t &from, [[maybe_unused]] const continuous_mode_t &to) const override
    {
        // For now, allow any switch. This can be customized later.
        return true;
    }

    resources_t get_switch_cost([[maybe_unused]] const state_t &current_state, [[maybe_unused]] const continuous_mode_t &from, [[maybe_unused]] const continuous_mode_t &to) const override
    {
        // For now, return zero resources. This can be customized later.
        return resources_t{0.0, 0.0};
    }
};

} // namespace heating

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

inline json::jnode_t &operator<<(json::jnode_t &lhs, const heating::discrete_search_t &rhs)
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

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, heating::discrete_search_t &rhs)
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

inline json::jnode_t &operator<<(json::jnode_t &lhs, const heating::continuous_search_t &rhs)
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

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, heating::continuous_search_t &rhs)
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
