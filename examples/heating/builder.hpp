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
        continous_mode_t mode;
        mode.id = id;

        // Define A, B, C, D matrices for a 1-state system
        // A = [-h * area / (mass * Cp0)]
        mode.system.A = {{-this->h * this->area / (this->mass * this->Cp0)}};

        // B = [eta * R0 / (mass * Cp0)]
        mode.system.B = {{this->eta * this->R0 / (this->mass * this->Cp0)}};

        // C = [1] (output is temperature)
        mode.system.C = {{1.0}};

        // D = [0]
        mode.system.D = {{0.0}};

        // Input for the mode (assuming a constant input power for this mode)
        // This input will be set by the main.cpp, so we can leave it as default or a placeholder.
        mode.input[0] = 1.0; // Placeholder input power

        return mode;
    }

    /// @brief Creates a discrete-time mode.
    inline auto make_discrete_mode(flexman::core::ModeId id, double sample_time) const noexcept
    {
        // First, create the continuous-time mode.
        continous_mode_t ct_mode = this->make_continuous_mode(id);
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
