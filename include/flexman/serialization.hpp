/// @file serialization.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Contains all the serialization procedures.

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
inline json::jnode_t &operator<<(json::jnode_t &lhs, const flexman::Mode<System, Input> &rhs)
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
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, flexman::Mode<System, Input> &rhs)
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
inline json::jnode_t &operator<<(json::jnode_t &lhs, const flexman::ParetoFront<State, Resources> &rhs)
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
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, flexman::ParetoFront<State, Resources> &rhs)
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
inline json::jnode_t &operator<<(json::jnode_t &lhs, const flexman::Result<State, Resources> &rhs)
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
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, flexman::Result<State, Resources> &rhs)
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
inline json::jnode_t &operator<<(json::jnode_t &lhs, const flexman::ModeExecution &rhs)
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
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, flexman::ModeExecution &rhs)
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
inline json::jnode_t &operator<<(json::jnode_t &lhs, const flexman::Solution<State, Resources> &rhs)
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
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, flexman::Solution<State, Resources> &rhs)
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
template <typename System, typename State, typename Input, typename Mode, class Resources>
inline json::jnode_t &operator<<(json::jnode_t &lhs, const flexman::Manager<State, Mode, Resources> &rhs)
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
template <typename System, typename State, typename Input, typename Mode, class Resources>
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, flexman::Manager<State, Mode, Resources> &rhs)
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
