/// @file common.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines common data structures for Particle Swarm Optimization (PSO).
///
/// @details
/// This file provides shared definitions for the PSO framework, including:
/// - The `SolverParameters` structure, which encapsulates key parameters
///   for controlling the optimization process.
///
/// The `SolverParameters` structure includes configurable options such as:
/// - The number of particles in the swarm.
/// - The maximum number of iterations.
/// - The inertia weight, affecting velocity retention.
/// - Cognitive and social coefficients, which balance exploration
///   and exploitation within the optimization process.
///
/// These parameters are essential for tuning PSO-based optimization
/// algorithms to achieve efficient convergence and exploration.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <timelib/timespec.hpp>

#include <algorithm>
#include <array>
#include <functional>

namespace flexman
{
namespace pso
{

/// @brief Structure to define PSO (Particle Swarm Optimization) parameters.
struct SolverParameters {
    /// @brief Maximum number of iterations.
    unsigned num_particles  = 100;
    /// @brief Maximum number of iterations.
    unsigned max_iterations = 50;
    /// @brief Weight for retaining previous velocity.
    double inertia          = 0.2;
    /// @brief Weight for personal best influence.
    double cognitive        = 0.4;
    /// @brief Weight for global best influence.
    double social           = 0.4;
};

} // namespace pso
} // namespace flexman
