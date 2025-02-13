/// @file defines.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines core types and system parameters for the tapping system.
///
/// @details
/// This file provides fundamental type definitions and constants used
/// throughout the tapping system, including:
/// - Definitions for system dimensions (states, inputs, outputs).
/// - Type aliases for state, input, solution, and optimization results.
/// - Representations for discrete and continuous state-space models.
/// - Mode definitions for both discrete and continuous systems.
/// - A `simulation_t` structure for managing simulation data and metadata.
///
/// These definitions serve as the foundation for modeling and optimizing
/// the tapping system within the Flexman framework.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "resources.hpp"

#include <fsmlib/control.hpp>

#include <flexman/core/mode.hpp>
#include <flexman/core/pareto_front.hpp>
#include <flexman/core/result.hpp>
#include <flexman/core/solution.hpp>
#include <flexman/simulation/common.hpp>

/// @brief A tapping system.
namespace tapping
{

/// @brief Number of system states.
constexpr std::size_t n_states = 3;

/// @brief Number of system inputs.
constexpr std::size_t n_input = 2;

/// @brief Number of system outputs.
constexpr std::size_t n_output = 3;

/// @brief State vector type.
using state_t = fsmlib::Vector<double, n_states>;

/// @brief Input vector type.
using input_t = fsmlib::Vector<double, n_input>;

/// @brief Result type containing a state and resource data.
using result_t = flexman::core::Result<state_t, resources_t>;

/// @brief Solution type containing a state and resource data.
using solution_t = flexman::core::Solution<state_t, resources_t>;

/// @brief Pareto front type for storing optimal solutions.
using pareto_front_t = flexman::core::ParetoFront<state_t, resources_t>;

/// @brief Discrete state-space system representation.
using discrete_system_t = fsmlib::control::DiscreteStateSpace<double, n_states, n_input, n_output>;

/// @brief Continuous state-space system representation.
using continous_system_t = fsmlib::control::StateSpace<double, n_states, n_input, n_output>;

/// @brief Mode representation for discrete systems.
using discrete_mode_t = flexman::core::Mode<discrete_system_t, input_t>;

/// @brief Mode representation for continuous systems.
using continous_mode_t = flexman::core::Mode<continous_system_t, input_t>;

/// @brief Simulation data and its associated name.
struct simulation_t {
    /// @brief Simulation data.
    flexman::simulation::Simulation<state_t, resources_t> data;
    /// @brief Name of the simulation.
    std::string name;
};

} // namespace tapping
