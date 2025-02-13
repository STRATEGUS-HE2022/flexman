/// @file logging.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines logging utilities for different components of the Flexman library.
///
/// @details
/// This file provides a set of loggers to track events and activities across
/// various components of the Flexman library. It includes:
/// - `solution`: Logs events related to solution evaluation and updates.
/// - `common`: Logs general-purpose events across different modules.
/// - `search`: Logs search process details and optimizations.
/// - `round`: Logs iteration details within search and optimization rounds.
/// - `pso`: Logs events related to Particle Swarm Optimization (PSO).
/// - `app`: Logs high-level application events.
///
/// The logging system is based on the Quire logging framework, providing
/// structured and configurable output for debugging and monitoring purposes.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <quire/quire.hpp>
#include <quire/registry.hpp>

namespace flexman
{

/// @brief Logging utilities for different components of the FlexMan library.
///
/// This namespace provides categorized loggers to track events related to
/// solution evaluation, search processes, optimization routines, and
/// general application execution. The logging system is structured to
/// offer configurable verbosity levels for debugging and monitoring.
namespace logging
{

/// @brief Logger for solution-related events.
[[maybe_unused]]
auto solution = quire::logger_t(
    "solution",
    quire::info,
    '|',
    {quire::option_t::time, quire::option_t::header, quire::option_t::level});

/// @brief Logger for common events.
[[maybe_unused]]
auto common = quire::logger_t(
    "common  ",
    quire::info,
    '|',
    {quire::option_t::time, quire::option_t::header, quire::option_t::level});

/// @brief Logger for search-related events.
[[maybe_unused]]
auto search = quire::logger_t(
    "search  ",
    quire::info,
    '|',
    {quire::option_t::time, quire::option_t::header, quire::option_t::level});

/// @brief Logger for round-related events.
[[maybe_unused]]
auto round = quire::logger_t(
    "round   ",
    quire::info,
    '|',
    {quire::option_t::time, quire::option_t::header, quire::option_t::level});

/// @brief Logger for PSO-related events.
[[maybe_unused]]
auto pso = quire::logger_t(
    "pso     ",
    quire::info,
    '|',
    {quire::option_t::time, quire::option_t::header, quire::option_t::level});

/// @brief Logger for application-level events.
[[maybe_unused]]
auto app = quire::logger_t(
    "app     ",
    quire::info,
    '|',
    {quire::option_t::time, quire::option_t::header, quire::option_t::level});

} // namespace logging
} // namespace flexman
