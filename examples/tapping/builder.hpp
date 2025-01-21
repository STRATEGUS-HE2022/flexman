/// @file builder.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include "parameters.hpp"
#include "defines.hpp"

namespace tapping
{

/// @brief Builds a mode.
struct builder_t : public parameters_t {
    builder_t(parameters_t params = parameters_t())
        : parameters_t(std::move(params))
    {
        // Nothing to do.
    }

    /// @brief Creates a continuous-time state space model.
    inline auto make_continuous_mode(flexman::mode_id_t id) const noexcept
    {
        // Rotations to depth.
        const auto R2D = (57.295779513 / 360) * Ts * Gr;
        // Create the mode.
        continous_mode_t mode;
        mode.id       = id;
        mode.input    = { V, Fs };
        mode.system.A = {
            { -Kb / J, Kt / J, -Fd * Gr / J },
            { -Ke / L, -R / L, 0.0 },
            { R2D, 0.0, 0.0 }
        };
        mode.system.B = {
            { 0, -Gr / J },
            { 1 / L, 0.0 },
            { 0.0, 0.0 },
        };
        mode.system.C = {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
        };
        mode.system.D = {
            { 0, 0 },
            { 0, 0 },
            { 0, 0 },
        };
        return mode;
    }

    /// @brief Creates a discrete-time mode.
    inline auto make_discrete_mode(flexman::mode_id_t id, double sample_time) const noexcept
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

} // namespace tapping
