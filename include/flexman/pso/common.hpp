/// @file common.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Data-structures related to PSO.

#pragma once

#include <timelib/timespec.hpp>

#include <functional>
#include <algorithm>
#include <array>

namespace flexman::pso
{

/// @brief Structure to define PSO (Particle Swarm Optimization) parameters.
struct SolverParameters {
    unsigned num_particles  = 100; ///< Maximum number of iterations.
    unsigned max_iterations = 50;  ///< Maximum number of iterations.
    double inertia          = 0.2; ///< Weight for retaining previous velocity.
    double cognitive        = 0.4; ///< Weight for personal best influence.
    double social           = 0.4; ///< Weight for global best influence.
};

} // namespace flexman::pso
