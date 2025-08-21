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

        // Parameter checks
        if (this->mass <= 0.0) {
            std::cerr << "mass must be > 0\n";
        }
        if (this->Cp0 <= 0.0) {
            std::cerr << "Cp0 must be > 0\n";
        }
        if (this->area <= 0.0) {
            std::cerr << "area must be > 0\n";
        }
        if (this->h < 0.0) {
            std::cerr << "h must be >= 0\n";
        }
        if (this->eta < 0.0 || this->eta > 1.0) {
            std::cerr << "eta outside [0,1]\n";
        }
        if (this->env_mass <= 0.0) {
            std::cerr << "env_mass must be > 0\n";
        }
        if (this->Cp_env <= 0.0) {
            std::cerr << "Cp_env must be > 0\n";
        }
        if (this->G_leak < 0.0) {
            std::cerr << "G_leak must be >= 0\n";
        }

        // Precompute conductances and heat capacities
        const double G     = this->h * this->area;
        const double Cx    = this->mass * this->Cp0;
        const double Ce    = this->env_mass * this->Cp_env;
        const double Gleak = this->G_leak;

        // A(2x2)
        mode.system.A = {
            {{-G / Cx}, {+G / Cx}},
            {{+G / Ce}, {-(G + Gleak) / Ce}},
        };

        // B(2x1): u = electrical power [W]
        mode.system.B = {
            {this->eta * this->R0 / Cx},
            {0.0},
        };

        // C(1x2): output T (workpiece); use {{ {1,0}, {0,1} }} if you want both outputs
        mode.system.C = {{{1.0}, {1.0}}};

        // D(1x1)
        mode.system.D = {{0.0}};

        // Placeholder input power
        mode.input[0] = 10000.0;

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
        // Return the discretized mode.
        return mode;
    }
};

} // namespace heating
