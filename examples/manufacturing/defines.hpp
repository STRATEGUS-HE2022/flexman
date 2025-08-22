/// @file defines.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Manufacturing system type definitions.
///
/// @details
/// This file defines the core types and constants for a multi-machine manufacturing
/// system where each mode represents a different machine operating on different
/// aspects of a workpiece. The system tracks multiple state variables representing
/// different manufacturing properties that can be modified by different machines.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <flexman/flexman.hpp>
#include <fsmlib/fsmlib.hpp>
#include <fsmlib_support.hpp>

#include "resources.hpp"

/// @brief A multi-machine manufacturing system.
namespace manufacturing
{

// Forward declarations
class discrete_mode_t;
class continuous_mode_t;

/// @brief Number of system states (representing different workpiece properties).
constexpr std::size_t n_states = 6;
/// @brief Number of system inputs.
constexpr std::size_t n_input  = 2;
/// @brief Number of system outputs.
constexpr std::size_t n_output = 6;

/// @brief State vector type representing workpiece properties:
/// [surface_roughness, hardness, temperature, dimensional_accuracy, coating_thickness, stress_level]
using state_t = fsmlib::Vector<double, n_states>;

/// @brief Input vector type representing machine operation parameters:
/// [primary_parameter, secondary_parameter]
using input_t = fsmlib::Vector<double, n_input>;

/// @brief Resources type representing manufacturing costs and time.
using resources_t = manufacturing::resources_t;

/// @brief Result type containing a state and resource data.
using result_t = flexman::core::Result<state_t, resources_t>;

/// @brief Solution type containing a state and resource data.
using solution_t = flexman::core::Solution<state_t, resources_t>;

/// @brief Pareto front type for storing optimal solutions.
using pareto_front_t = flexman::core::ParetoFront<state_t, resources_t>;

/// @brief Discrete state-space system representation.
using discrete_system_t = fsmlib::control::DiscreteStateSpace<double, n_states, n_input, n_output>;

/// @brief Continuous state-space system representation.
using continuous_system_t = fsmlib::control::StateSpace<double, n_states, n_input, n_output>;

/// @brief Simulation data and its associated name.
struct simulation_t {
    /// @brief The simulation data.
    flexman::simulation::Simulation<state_t, resources_t> data;
    /// @brief The name associated with the simulation.
    std::string name;
};

/// @brief Machine types in the manufacturing system.
enum class MachineType {
    MillingMachine    = 0, ///< Milling machine for surface finishing and dimensional accuracy
    HeatTreatment     = 1, ///< Heat treatment furnace for hardness and stress relief
    CoatingStation    = 2, ///< Coating application station
    PolishingMachine  = 3, ///< Polishing machine for surface roughness improvement
    QualityInspection = 4, ///< Quality inspection and measurement station
    CoolingStation    = 5  ///< Cooling station for temperature control
};

/// @brief Machine operation modes.
enum class OperationMode {
    Light     = 0, ///< Light operation mode - lower impact, lower cost, faster
    Standard  = 1, ///< Standard operation mode - balanced performance
    Intensive = 2  ///< Intensive operation mode - high impact, higher cost, slower
};

} // namespace manufacturing
