/// @file builder.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines a builder for creating continuous and discrete modes.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "defines.hpp"
#include "parameters.hpp"

namespace heating
{

/// @brief Mode representation for discrete systems.
class discrete_mode_t : public flexman::core::Mode<discrete_system_t, input_t>
{
public:
    parameters_t parameters;       ///< Parameters for the discrete mode.
    double total_electrical_power; ///< Pre-computed total electrical power (heater + fan) [W]

    /// @brief Default constructor.
    discrete_mode_t() = default;
};

/// @brief Mode representation for continuous systems.
// using continuous_mode_t = flexman::core::Mode<continuous_system_t, input_t>;
class continuous_mode_t : public flexman::core::Mode<continuous_system_t, input_t>
{
public:
    parameters_t parameters;       ///< Parameters for the continuous mode.
    double total_electrical_power; ///< Pre-computed total electrical power (heater + fan) [W]

    /// @brief Default constructor.
    continuous_mode_t() = default;
};

/// @brief Builds a mode.
struct builder_t : public parameters_t {
    builder_t(parameters_t params = parameters_t())
        : parameters_t(std::move(params))
    {
        // Nothing to do.
    }

    /// @brief Creates a continuous-time state space model.
    inline auto make_continuous_mode(flexman::core::ModeId id) const noexcept
    {
        continuous_mode_t mode;
        mode.id = id;

        // Parameter checks.
        assert(this->mass > 0.0);
        assert(this->Cp0 > 0.0);
        assert(this->area > 0.0);
        assert(this->h >= 0.0);
        assert(this->eta >= 0.0 && this->eta <= 1.0);
        assert(this->env_mass > 0.0);
        assert(this->Cp_env > 0.0);
        assert(this->G_leak0 >= 0.0);
        assert(this->k_leak_per_w >= 0.0);
        assert(this->G_leak_min >= 0.0);
        assert(this->G_leak_max >= this->G_leak_min);
        assert(this->input_power >= 0.0);

        // Per-step update of A using G_leak(u) = G0 + k_leak * u
        const double u  = input_power;
        const double G  = this->h * this->area;
        const double Cx = this->mass * this->Cp0;
        const double Ce = this->env_mass * this->Cp_env;

        // Pre-compute leak conductance and fan power for this fixed input power
        const double G0     = this->G_leak0;      // base leak [W/°C]
        const double k_leak = this->k_leak_per_w; // [ (W/°C) per W of heater power ]
        const double Gmin   = this->G_leak_min;
        const double Gmax   = this->G_leak_max;

        const double G_leak_eff = std::clamp(G0 + k_leak * u, Gmin, Gmax);

        // Pre-compute fan power for this mode (embedded in the system)
        const double Pf0    = this->fan_P0;      // [W] idle fan draw
        const double Pf_cub = this->fan_P_cubic; // [W] scale factor for cubic term
        const double ratio  = std::max(G_leak_eff / std::max(G0, 1e-9), 0.0);
        const double P_fan  = Pf0 + Pf_cub * ratio * ratio * ratio;

        // Store total electrical power (heater + fan) for this mode
        mode.total_electrical_power = u + P_fan;

        // A(2x2) - State transition matrix for thermal system
        // x = [T_workpiece, T_environment]
        // Heat flows: workpiece ↔ environment ↔ room (with variable leak)
        mode.system.A = {
            {{-G / Cx}, {+G / Cx}},
            {{+G / Ce}, {-(G + G_leak_eff) / Ce}},
        };

        // B(2x1): u = electrical power [W]
        mode.system.B = {
            {this->eta / Cx},
            {0.0},
        };

        // C(1x2): output T (workpiece); use {{ {1,0}, {0,1} }} if you want both outputs
        mode.system.C = {
            {{1.0}, {0.0}},
        };

        // D(1x1)
        mode.system.D = {{0.0}};

        mode.parameters = *this;

        return mode;
    }

    /// @brief Creates a discrete-time mode.
    inline auto make_discrete_mode(flexman::core::ModeId id, double sample_time) const noexcept
    {
        // First, create the continuous-time mode.
        continuous_mode_t ct_mode = this->make_continuous_mode(id);
        // Create the discrete-time mode
        discrete_mode_t mode;
        // Initialize the discrete-time mode.
        mode.id     = id;
        mode.input  = ct_mode.input;
        mode.system = fsmlib::control::c2d(ct_mode.system, sample_time);

        mode.parameters             = *this;
        mode.total_electrical_power = ct_mode.total_electrical_power; // Copy from continuous mode

        // Return the discretized mode.
        return mode;
    }
};

inline std::ostream &operator<<(std::ostream &lhs, const discrete_mode_t &rhs)
{
    using namespace fsmlib;
    lhs << "Mode ID: " << rhs.id << "\n";
    lhs << "System:\n";
    lhs << rhs.system << "\n";
    lhs << "Input: ";
    lhs << rhs.input << "\n";
    lhs << "Total Electrical Power: " << rhs.total_electrical_power << " W\n";
    lhs << "Parameters: ";
    lhs << rhs.parameters << "\n";
    return lhs;
}

inline std::ostream &operator<<(std::ostream &lhs, const continuous_mode_t &rhs)
{
    lhs << "Mode ID: " << rhs.id << "\n";
    lhs << "System:\n";
    lhs << rhs.system << "\n";
    lhs << "Input:\n";
    lhs << rhs.input << "\n";
    lhs << "Total Electrical Power: " << rhs.total_electrical_power << " W\n";
    lhs << "Parameters:\n";
    lhs << rhs.parameters << "\n";
    return lhs;
}

} // namespace heating

namespace json
{

inline json::jnode_t &operator<<(json::jnode_t &lhs, const heating::discrete_mode_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["id"] << rhs.id;
    lhs["system"] << rhs.system;
    lhs["input"] << rhs.input;
    lhs["parameters"] << rhs.parameters;
    lhs["total_electrical_power"] << rhs.total_electrical_power;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, heating::discrete_mode_t &rhs)
{
    lhs["id"] >> rhs.id;
    lhs["system"] >> rhs.system;
    lhs["input"] >> rhs.input;
    lhs["parameters"] >> rhs.parameters;
    lhs["total_electrical_power"] >> rhs.total_electrical_power;
    return lhs;
}

inline json::jnode_t &operator<<(json::jnode_t &lhs, const heating::continuous_mode_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["id"] << rhs.id;
    lhs["system"] << rhs.system;
    lhs["input"] << rhs.input;
    lhs["parameters"] << rhs.parameters;
    lhs["total_electrical_power"] << rhs.total_electrical_power;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, heating::continuous_mode_t &rhs)
{
    lhs["id"] >> rhs.id;
    lhs["system"] >> rhs.system;
    lhs["input"] >> rhs.input;
    lhs["parameters"] >> rhs.parameters;
    lhs["total_electrical_power"] >> rhs.total_electrical_power;
    return lhs;
}

} // namespace json
