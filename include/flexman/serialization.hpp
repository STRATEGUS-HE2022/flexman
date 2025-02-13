/// @file serialization.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Implements serialization and deserialization procedures for Flexman objects.
///
/// @details
/// This file provides serialization and deserialization operators for key
/// data structures within the Flexman library. The supported types include:
/// - `Mode`
/// - `ModeExecution`
/// - `Solution`
/// - `ParetoFront`
/// - `Result`
/// - `Manager`
///
/// These operators facilitate seamless conversion between JSON representations
/// and Flexman objects, enabling efficient data storage, logging, and
/// exchange of optimization results.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "flexman/flexman.hpp"

#include <json/json.hpp>

namespace json
{

/// @brief Serializes a Mode object to a JSON node.
///
/// @tparam System The type representing the system.
/// @tparam Input The type representing the input.
///
/// @param lhs The JSON node to write to.
/// @param rhs The Mode object to serialize.
///
/// @return A reference to the updated JSON node.
template <typename System, typename Input>
inline auto operator<<(json::jnode_t &lhs, const flexman::core::Mode<System, Input> &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["id"] << rhs.id;
    lhs["system"] << rhs.system;
    lhs["input"] << rhs.input;
    return lhs;
}

/// @brief Deserializes a Mode object from a JSON node.
///
/// @tparam System The type representing the system.
/// @tparam Input The type representing the input.
///
/// @param lhs The JSON node to read from.
/// @param rhs The Mode object to populate.
///
/// @return A reference to the original JSON node.
template <typename System, typename Input>
inline auto operator>>(const json::jnode_t &lhs, flexman::core::Mode<System, Input> &rhs) -> const json::jnode_t &
{
    lhs["id"] >> rhs.id;
    lhs["system"] >> rhs.system;
    lhs["input"] >> rhs.input;
    return lhs;
}

/// @brief Serializes a ParetoFront object to a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to write to.
/// @param rhs The ParetoFront object to serialize.
///
/// @return A reference to the updated JSON node.
template <typename State, typename Resources>
inline auto operator<<(json::jnode_t &lhs, const flexman::core::ParetoFront<State, Resources> &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["solutions"] << rhs.solutions;
    lhs["step_length"] << rhs.step_length;
    lhs["steps_per_iteration"] << rhs.steps_per_iteration;
    lhs["iteration"] << rhs.iteration;
    lhs["runtime"] << rhs.runtime;
    return lhs;
}

/// @brief Deserializes a ParetoFront object from a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to read from.
/// @param rhs The ParetoFront object to populate.
///
/// @return A reference to the original JSON node.
template <typename State, typename Resources>
inline auto operator>>(const json::jnode_t &lhs, flexman::core::ParetoFront<State, Resources> &rhs)
    -> const json::jnode_t &
{
    lhs["solutions"] >> rhs.solutions;
    lhs["step_length"] >> rhs.step_length;
    lhs["step"] >> rhs.step;
    lhs["runtime"] >> rhs.runtime;
    return lhs;
}

/// @brief Serializes a Result object to a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to write to.
/// @param rhs The Result object to serialize.
///
/// @return A reference to the updated JSON node.
template <typename State, typename Resources>
inline auto operator<<(json::jnode_t &lhs, const flexman::core::Result<State, Resources> &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["pareto_fronts"] << rhs.pareto_fronts;
    return lhs;
}

/// @brief Deserializes a Result object from a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to read from.
/// @param rhs The Result object to populate.
///
/// @return A reference to the original JSON node.
template <typename State, typename Resources>
inline auto operator>>(const json::jnode_t &lhs, flexman::core::Result<State, Resources> &rhs) -> const json::jnode_t &
{
    lhs["pareto_fronts"] >> rhs.pareto_fronts;
    return lhs;
}

/// @brief Serializes a ModeExecution object to a JSON node.
///
/// @param lhs The JSON node to write to.
/// @param rhs The ModeExecution object to serialize.
///
/// @return A reference to the updated JSON node.
inline auto operator<<(json::jnode_t &lhs, const flexman::core::ModeExecution &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["mode"] << rhs.mode;
    lhs["times"] << rhs.times;
    return lhs;
}

/// @brief Deserializes a ModeExecution object from a JSON node.
///
/// @param lhs The JSON node to read from.
/// @param rhs The ModeExecution object to populate.
///
/// @return A reference to the original JSON node.
inline auto operator>>(const json::jnode_t &lhs, flexman::core::ModeExecution &rhs) -> const json::jnode_t &
{
    std::vector<std::string> sequence;
    lhs["mode"] >> rhs.mode;
    lhs["times"] >> rhs.times;
    return lhs;
}

/// @brief Serializes a Solution object to a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to write to.
/// @param rhs The Solution object to serialize.
///
/// @return A reference to the updated JSON node.
template <typename State, typename Resources>
inline auto operator<<(json::jnode_t &lhs, const flexman::core::Solution<State, Resources> &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["sequence"] << rhs.sequence;
    lhs["state"] << rhs.state;
    lhs["resources"] << rhs.resources;
    return lhs;
}

/// @brief Deserializes a Solution object from a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to read from.
/// @param rhs The Solution object to populate.
///
/// @return A reference to the original JSON node.
template <typename State, typename Resources>
inline auto operator>>(const json::jnode_t &lhs, flexman::core::Solution<State, Resources> &rhs)
    -> const json::jnode_t &
{
    std::vector<std::string> sequence;
    lhs["sequence"] >> rhs.sequence;
    lhs["state"] >> rhs.state;
    lhs["resources"] >> rhs.resources;
    return lhs;
}

/// @brief Serializes a Manager object to a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to write to.
/// @param rhs The Manager object to serialize.
///
/// @return A reference to the updated JSON node.
template <typename State, typename Mode, class Resources>
inline auto operator<<(json::jnode_t &lhs, const flexman::core::Manager<State, Mode, Resources> &rhs) -> json::jnode_t &
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["initial_state"] << rhs.initial_state;
    lhs["target_state"] << rhs.target_state;
    lhs["time_delta"] << rhs.time_delta;
    lhs["time_max"] << rhs.time_max;
    lhs["threshold"] << rhs.threshold;
    lhs["timeout"] << rhs.timeout;
    lhs["interactive"] << rhs.interactive;
    return lhs;
}

/// @brief Deserializes a Manager object from a JSON node.
///
/// @tparam State The type representing the state.
/// @tparam Mode The type representing the mode.
/// @tparam Resources The type representing the resources.
///
/// @param lhs The JSON node to read from.
/// @param rhs The Manager object to populate.
///
/// @return A reference to the original JSON node.
template <typename State, typename Mode, class Resources>
inline auto operator>>(const json::jnode_t &lhs, flexman::core::Manager<State, Mode, Resources> &rhs)
    -> const json::jnode_t &
{
    lhs["initial_state"] >> rhs.initial_state;
    lhs["target_state"] >> rhs.target_state;
    lhs["time_delta"] >> rhs.time_delta;
    lhs["time_max"] >> rhs.time_max;
    lhs["threshold"] >> rhs.threshold;
    lhs["timeout"] >> rhs.timeout;
    lhs["interactive"] >> rhs.interactive;
    return lhs;
}

} // namespace json
