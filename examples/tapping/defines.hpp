/// @file defines.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#pragma once

#include "resources.hpp"

#include <fsmlib/control.hpp>

#include <flexman/data_structure/mode.hpp>
#include <flexman/data_structure/result.hpp>
#include <flexman/data_structure/solution.hpp>
#include <flexman/data_structure/pareto_front.hpp>
#include <flexman/simulation/common.hpp>

namespace tapping
{

constexpr std::size_t n_states = 3;
constexpr std::size_t n_input  = 2;
constexpr std::size_t n_output = 3;

using state_t        = fsmlib::Vector<double, n_states>;
using input_t        = fsmlib::Vector<double, n_input>;
using result_t       = flexman::Result<state_t, resources_t>;
using solution_t     = flexman::Solution<state_t, resources_t>;
using pareto_front_t = flexman::ParetoFront<state_t, resources_t>;

using discrete_system_t  = fsmlib::control::DiscreteStateSpace<double, n_states, n_input, n_output>;
using continous_system_t = fsmlib::control::StateSpace<double, n_states, n_input, n_output>;

using discrete_mode_t  = flexman::Mode<discrete_system_t, input_t>;
using continous_mode_t = flexman::Mode<continous_system_t, input_t>;

struct simulation_t {
    flexman::simulation::Simulation<state_t, resources_t> data;
    std::string name;
};

} // namespace tapping
