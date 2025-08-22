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

/// @brief Outputs the SolverParameters object to an output stream.
/// @param lhs The output stream to write to.
/// @param rhs The SolverParameters object to output.
/// @return A reference to the output stream.
std::ostream &operator<<(std::ostream &lhs, const SolverParameters &rhs)
{
    lhs << "SolverParameters{";
    lhs << "num_particles: " << rhs.num_particles << ", ";
    lhs << "max_iterations: " << rhs.max_iterations << ", ";
    lhs << "inertia: " << rhs.inertia << ", ";
    lhs << "cognitive: " << rhs.cognitive << ", ";
    lhs << "social: " << rhs.social;
    lhs << "}";
    return lhs;
}

} // namespace pso
} // namespace flexman
