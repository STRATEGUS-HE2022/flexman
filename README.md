# FlexMan: Adaptive Scheduling and Optimization Library

[![Ubuntu](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/ubuntu.yml)
[![MacOS](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/macos.yml/badge.svg)](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/macos.yml)
[![Documentation](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/documentation.yml/badge.svg)](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/documentation.yml)

This project is a generalized simulation and optimization library designed to
solve continuous-time scheduling problems by finding the best sequence of modes
to achieve a target state. Users can define their systems' state evolution
freely, track resources, and optimize performance using techniques like
exhaustive searches, heuristic evaluations, and Particle Swarm Optimization
(PSO). The library is modular and adaptable to various applications, with the
industrial tapping machine serving as an example.

## Table of Contents

- Features
- Installation
- Usage example: Industrial Tapping Machine
- Code Structure
- How to Run the Example
- Extending the Library
- Contributing
- Acknowledgments
- License

## Features

- **Generalized Scheduling**: Solve continuous-time scheduling problems by
  evolving systems towards a target state.
- **Simulation Managers**: Customizable discrete and continuous system
  evolution.
- **Resource Tracking**: Define and monitor multi-dimensional metrics such as
  energy and time during the simulation.
- **Pareto Optimization**: Analyze trade-offs between performance metrics.
- **Post-Search Optimization**: Fine-tune results using Particle Swarm
  Optimization (PSO).
- **Customizable Search Strategies**: Supports exhaustive searches to explore
  all possibilities and heuristic searches for faster, approximate solutions.
- **Customizable Comparison Logic**: Define `is_strictly_better_than` and
  `is_probably_better_than` to suit your problem domain.
- **Modularity**: Easily adaptable to other use cases and systems.

## Installation

This library is header-only and does not require installation. All dependencies
are automatically fetched using the provided CMake configuration.

Clone the repository:

```bash
git clone https://github.com/username/repository.git
cd repository
```

Build the project using CMake:

```bash
mkdir build && cd build
cmake ..
make
```

## Usage example: Industrial Tapping Machine

The provided example focuses on simulating and optimizing the operations of an
industrial tapping machine. The simulation incorporates:

- **Discrete and Continuous Search Managers**: Simulate system evolution for
  discrete and continuous state spaces.
- **Resources**: Define multi-dimensional metrics, such as energy and time, that
  guide the search by comparing solutions.
- **State System**: Define system states, inputs, and outputs.
- **Builder**: Construct modes for the system.
- **PSO Optimization**: Refine solutions to achieve optimal performance.

### Step 1: Define the System

Set up the state, input, and resource types for the simulation:

```cpp
namespace tapping {
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
} // namespace tapping
```

The `resources_t` struct allows users to define multiple metrics for optimization. In this example, it tracks energy and time, but it can include other dimensions depending on the use case. These metrics are essential for guiding the search, as they define what makes one solution better than another.

```cpp
/// @brief Tapping resources.
struct resources_t {
    double energy; ///< Energy spent tapping.
    double time;   ///< Time spent tapping.
};
```

### Step 2: Create Discrete and Continuous Modes

Use a `builder_t` class to define continuous and discrete modes. For instance,
in the tapping example we create a continuous-time mode this way:

```cpp
/// @brief Creates a continuous-time state space model.
inline auto make_continuous_mode(flexman::ModeId id) const noexcept
{
    continous_mode_t mode;
    mode.id       = id;
    mode.input    = { ... };
    mode.system.A = { ... };
    mode.system.B = { ... };
    mode.system.C = { ... };
    mode.system.D = { ... };
    return mode;
}
```

and if the library you are using for defining the mode allows to discretize it
you can also add a `make_discrete_mode` function, like we have done with the
tapping example:

```cpp
/// @brief Creates a discrete-time mode.
inline auto make_discrete_mode(flexman::ModeId id, double sample_time) const noexcept
{
    // First, create the continuous-time mode.
    continous_mode_t ct_mode = this->make_continuous_mode(id);
    // Create the discrete-time mode
    discrete_mode_t mode;
    // Initialize the discrete-time mode.
    mode.id     = id;
    mode.input  = ct_mode.input;
    mode.system = fsmlib::control::c2d(ct_mode.system, sample_time);
    // Return the discretized mode.
    return mode;
}
```

Then you can build the modes, ready to be used by the search function:

```cpp
builder_t builder;
auto continuous_mode = builder.make_continuous_mode(1);
auto discrete_mode = builder.make_discrete_mode(1, 0.01);
```

### Step 3: Simulate Using Managers

Leverage `discrete_search_t` and `continuous_search_t` to simulate the system:

```cpp
tapping::discrete_search_t   discrete_manager;
tapping::continuous_search_t continuous_manager;

discrete_manager.updated_solution(solution, discrete_mode);
continuous_manager.updated_solution(solution, continuous_mode);
```

The library supports **exhaustive searches**, which explore all possible
solutions, and **heuristic searches**, which approximate the best solutions
efficiently. Users must define their logic for comparing solutions by
implementing two key functions:

- `is_strictly_better_than`: Determines if one solution is definitively better
  than another.
- `is_probably_better_than`: Provides a heuristic comparison for approximate
  searches.

### Step 4: Optimize with PSO

Refine results using Particle Swarm Optimization:

```cpp
// Assuming `result` contains initial solutions
auto optimized_result = pso::optimize(result);
```

## How to Run the Example

Compile the code:

```bash
mkdir build && cd build
cmake ..
make
```

Call the executable with `--help` to see all the settings we can set:

```bash
./flexman_tapping --help
```

This is an example of actual simulation:

```bash
./flexman_tapping --run 0 --mode 0 --algorithm 0 --pso --output output.json --plot
```

This command runs:

- **Run**: Runs a search (`--run 0`)
- **Mode**: Discrete mode (`--mode 0`)
- **Algorithm**: The search uses the heuristic algorithm (`--algorithm 0`)
- **Post-search optimization**: Enabled PSO search with default parameters (`--pso`)
- **Output**: The whole problem configuration as well as the results are saved to `output.json` (`--output output.json`)
- **Plot results**: Enabled (`--plot`)

View the results, including Pareto fronts and optimized solutions.

## Extending the Library

The library is modular and can be adapted for various systems by defining custom:

- State-space models
- Resource tracking metrics
- Optimization criteria

## Contributing

We welcome contributions! Please submit issues and pull requests on GitHub to help improve the project.

## Acknowledgments

Special thanks to **Michael Balszun** for contributions and inspiration.

## License

This project is licensed under the [BSD-2 License](LICENSE.md).
