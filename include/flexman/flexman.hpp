/// @file flexman.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief The main header file for the Flexman library.
///
/// @details
/// This file serves as the primary entry point for the Flexman library,
/// providing access to its core components, including:
/// - Data structures for managing modes, solutions, Pareto fronts, and results.
/// - Particle Swarm Optimization (PSO) utilities for optimizing mode execution sequences.
/// - Search algorithms for systematically exploring and refining solutions.
/// - Simulation functions for evaluating state evolution and mode transitions.
///
/// By including this header, users gain access to all major functionalities
/// within the Flexman library, enabling efficient optimization and decision-making
/// in simulation-based environments.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

/// @brief The root namespace for the FlexMan library.
///
/// @details This namespace encapsulates all core functionalities of the
/// FlexMan library, including search algorithms, simulation tools,
/// optimization techniques, logging utilities, and data structures.
/// It provides a structured framework for managing flexible
/// manufacturing processes and decision-making tasks.
namespace flexman
{
enum : unsigned char {
    FLEXMAN_MAJOR_VERSION = 1, ///< Major version of the library.
    FLEXMAN_MINOR_VERSION = 0, ///< Minor version of the library.
    FLEXMAN_MICRO_VERSION = 0  ///< Micro version of the library.
};
} // namespace flexman

#include "flexman/core/manager.hpp"
#include "flexman/core/mode.hpp"
#include "flexman/core/mode_execution.hpp"
#include "flexman/core/pareto_front.hpp"
#include "flexman/core/result.hpp"
#include "flexman/core/solution.hpp"

#include "flexman/pso/common.hpp"
#include "flexman/pso/optimize.hpp"

#include "flexman/search/common.hpp"
#include "flexman/search/search.hpp"

#include "flexman/simulation/common.hpp"
#include "flexman/simulation/simulate.hpp"
