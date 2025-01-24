# FlexMan: Adaptive Scheduling and Optimization Library

[![Ubuntu](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/ubuntu.yml)
[![Windows](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/windows.yml/badge.svg)](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/windows.yml)
[![MacOS](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/macos.yml/badge.svg)](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/macos.yml)
[![Documentation](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/documentation.yml/badge.svg)](https://github.com/STRATEGUS-HE2022/flexman/actions/workflows/documentation.yml)

This project is a generalized simulation and optimization library designed to solve continuous-time scheduling problems by finding the best sequence of modes to achieve a target state. Users can define their systems' state evolution freely, track resources, and optimize performance using techniques like exhaustive searches, heuristic evaluations, and Particle Swarm Optimization (PSO). The library is modular and adaptable to various applications, with the industrial tapping machine serving as an example.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage example: Industrial Tapping Machine](#usage-example-industrial-tapping-machine)
  - [Step 1: Define the System](#step-1-define-the-system)
  - [Step 2: Create Modes](#step-2-create-modes)
  - [Step 3: Simulate Using Managers](#step-3-simulate-using-managers)
  - [Step 4: Optimize with PSO](#step-4-optimize-with-pso)
- [Code Structure](#code-structure)
  - [Managers](#managers)
  - [Resources](#resources)
  - [Modes](#modes)
  - [Builder](#builder)
  - [PSO Integration](#pso-integration)
- [How to Run the Example](#how-to-run-the-example)
- [Extending the Library](#extending-the-library)
- [Contributing](#contributing)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## Features

- **Generalized Scheduling**: Solve continuous-time scheduling problems by evolving systems towards a target state.
- **Simulation Managers**: Customizable discrete and continuous system evolution.
- **Resource Tracking**: Define and monitor multi-dimensional metrics such as energy and time during the simulation.
- **Pareto Optimization**: Analyze trade-offs between performance metrics.
- **Post-Search Optimization**: Fine-tune results using Particle Swarm Optimization (PSO).
- **Customizable Search Strategies**: Supports exhaustive searches to explore all possibilities and heuristic searches for faster, approximate solutions.
- **Customizable Comparison Logic**: Define `is_strictly_better_than` and `is_probably_better_than` to suit your problem domain.
- **Modularity**: Easily adaptable to other use cases and systems.

## Installation

This library is header-only and does not require installation. All dependencies are automatically fetched using the provided CMake configuration.

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

The provided example focuses on simulating and optimizing the operations of an industrial tapping machine. The simulation incorporates:

- **Discrete and Continuous Search Managers**: Simulate system evolution for discrete and continuous state spaces.
- **Resources**: Define multi-dimensional metrics, such as energy and time, that guide the search by comparing solutions.
- **State System**: Define system states, inputs, and outputs.
- **Builder**: Construct modes for the system.
- **PSO Optimization**: Refine solutions to achieve optimal performance.

### Step 1: Define the System

Set up the state, input, and resource types for the simulation:

```cpp
namespace tapping {
    constexpr std::size_t n_states = 3;
    constexpr std::size_t n_input  = 2;
    constexpr std::size_t n_output = 3;

    using state_t = fsmlib::Vector<double, n_states>;
    using input_t = fsmlib::Vector<double, n_input>;
    using resources_t = flexman::resources_t;
    using solution_t = flexman::Solution<state_t, resources_t>;
}
```

The `resources_t` struct allows users to define multiple metrics for optimization. In this example, it tracks energy and time, but it can include other dimensions depending on the use case. These metrics are essential for guiding the search, as they define what makes one solution better than another.

### Step 2: Create Modes

Use the `builder_t` class to define continuous and discrete modes:

```cpp
builder_t builder;
auto continuous_mode = builder.make_continuous_mode(1);
auto discrete_mode = builder.make_discrete_mode(1, 0.01);
```

### Step 3: Simulate Using Managers

Leverage `discrete_search_t` and `continuous_search_t` to simulate the system:

```cpp
tapping::discrete_search_t discrete_manager;
tapping::continuous_search_t continuous_manager;

discrete_manager.updated_solution(solution, discrete_mode);
continuous_manager.updated_solution(solution, continuous_mode);
```

The library supports **exhaustive searches**, which explore all possible solutions, and **heuristic searches**, which approximate the best solutions efficiently. Users must define their logic for comparing solutions by implementing two key functions:

- `is_strictly_better_than`: Determines if one solution is definitively better than another.
- `is_probably_better_than`: Provides a heuristic comparison for approximate searches.

### Step 4: Optimize with PSO

Refine results using Particle Swarm Optimization:

```cpp
// Assuming `result` contains initial solutions
auto optimized_result = pso::optimize(result);
```

## Code Structure

### Managers

- **`discrete_search_t`**: Handles discrete-time simulations.
- **`continuous_search_t`**: Handles continuous-time simulations with Runge-Kutta integration.

### Resources

Defines energy and time tracking, or other user-defined metrics:

```cpp
struct resources_t {
    double energy;
    double time;
};
```

### Modes

Defines discrete and continuous state-space models:

```cpp
using discrete_mode_t = flexman::Mode<discrete_system_t, input_t>;
using continous_mode_t = flexman::Mode<continous_system_t, input_t>;
```

### Builder

Constructs modes for simulation:

```cpp
builder_t builder;
auto mode = builder.make_discrete_mode(1, 0.01);
```

### PSO Integration

Fine-tunes solutions post-search:

```cpp
result_t optimized_result = pso::optimize(initial_result);
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
