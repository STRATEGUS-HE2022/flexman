/// @file logging.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include <quire/registry.hpp>
#include <quire/quire.hpp>

namespace flexman::logging
{
/// @brief Logger for solution-related events.
[[maybe_unused]] auto solution = quire::logger_t("solution", quire::info, '|', { quire::option_t::time, quire::option_t::header, quire::option_t::level });

/// @brief Logger for common events.
[[maybe_unused]] auto common   = quire::logger_t("common  ", quire::info, '|', { quire::option_t::time, quire::option_t::header, quire::option_t::level });

/// @brief Logger for search-related events.
[[maybe_unused]] auto search   = quire::logger_t("search  ", quire::info, '|', { quire::option_t::time, quire::option_t::header, quire::option_t::level });

/// @brief Logger for round-related events.
[[maybe_unused]] auto round    = quire::logger_t("round   ", quire::info, '|', { quire::option_t::time, quire::option_t::header, quire::option_t::level });

/// @brief Logger for PSO-related events.
[[maybe_unused]] auto pso      = quire::logger_t("pso     ", quire::info, '|', { quire::option_t::time, quire::option_t::header, quire::option_t::level });

/// @brief Logger for application-level events.
[[maybe_unused]] auto app      = quire::logger_t("app     ", quire::info, '|', { quire::option_t::time, quire::option_t::header, quire::option_t::level });

} // namespace flexman::logging
