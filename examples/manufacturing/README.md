# Multi-Machine Manufacturing System Example

This example demonstrates a comprehensive multi-machine manufacturing system where each mode represents a different machine operating on different aspects of a workpiece. Unlike the heating and tapping examples where different modes represent different operating parameters for the same type of system, this example showcases completely independent machines working together in a manufacturing process.

## System Overview

The manufacturing system includes six different machine types:

1. **Milling Machine**: Affects surface roughness and dimensional accuracy
2. **Heat Treatment Furnace**: Affects hardness and stress relief
3. **Coating Station**: Applies protective coatings
4. **Polishing Machine**: Improves surface finish
5. **Quality Inspection Station**: Measurement and verification
6. **Cooling Station**: Temperature control

Each machine has three operation modes:

- **Light**: Faster, cheaper, less effective
- **Standard**: Balanced performance
- **Intensive**: Slower, more expensive, more effective

## State Vector

The workpiece is represented by a 6-dimensional state vector:

```
[surface_roughness, hardness, temperature, dimensional_accuracy, coating_thickness, stress_level]
```

Each machine affects different combinations of these properties:

- **Milling**: Reduces surface roughness and improves dimensional accuracy, but increases temperature and stress
- **Heat Treatment**: Significantly increases hardness, relieves stress, but raises temperature
- **Coating**: Adds coating thickness, slightly affects surface roughness
- **Polishing**: Dramatically improves surface finish, may remove some coating
- **Quality Inspection**: Provides measurement (minimal direct effects)
- **Cooling**: Reduces temperature and stress

## Resource Model

The system tracks multiple cost components:

- **Time**: Total processing time
- **Energy Cost**: Energy consumption cost
- **Material Cost**: Material and consumables cost
- **Labor Cost**: Labor cost
- **Equipment Wear**: Equipment maintenance cost
- **Quality Loss**: Quality degradation penalty

## Usage Examples

### Basic Search (Discrete Mode)

```bash
./flexman_manufacturing --mode 0 --algorithm 0 --iterations 8
```

### Search with Plotting

```bash
./flexman_manufacturing --mode 0 --algorithm 0 --iterations 8 --plot
```

### Simulation Mode

```bash
./flexman_manufacturing --run 1 --mode 0 --plot
```

### Custom Initial and Target States

```bash
./flexman_manufacturing \
  --initial_surface_roughness 15.0 \
  --initial_hardness 25.0 \
  --initial_temperature 30.0 \
  --target_surface_roughness 1.0 \
  --target_hardness 50.0 \
  --target_temperature 25.0 \
  --plot
```

### With Post-Search Optimization (PSO)

```bash
./flexman_manufacturing --mode 0 --algorithm 0 --pso --plot
```

## Key Features

### Independent Machine Models

Each machine has completely different:

- Physical effects on the workpiece
- Cost structures
- Processing rates
- Quality factors

### Multi-Objective Optimization

The system optimizes for:

- Minimal total processing time
- Minimal total cost (energy + material + labor + wear + quality loss)
- Achievement of target workpiece properties

### Realistic Manufacturing Constraints

- Machine-specific parameter ranges
- Cross-coupling effects between workpiece properties
- Operation mode trade-offs (speed vs. quality vs. cost)

### Comprehensive Analysis

The example provides:

- Pareto front visualization (time vs. cost)
- Detailed cost breakdown analysis
- Machine utilization statistics
- State evolution tracking during simulation

## Command Line Options

### Run Modes

- `-r 0` or `--run 0`: Search for optimal sequences
- `-r 1` or `--run 1`: Simulate individual machine operations

### System Modes

- `-m 0` or `--mode 0`: Discrete-time system
- `-m 1` or `--mode 1`: Continuous-time system

### Search Algorithms

- `-a 0` or `--algorithm 0`: Heuristic search (recommended)
- `-a 1` or `--algorithm 1`: Exhaustive search
- `-a 2` or `--algorithm 2`: Single machine analysis

### Initial State (workpiece starting conditions)

- `--initial_surface_roughness`: Surface roughness in μm (default: 10.0)
- `--initial_hardness`: Hardness in HRC (default: 20.0)
- `--initial_temperature`: Temperature in °C (default: 25.0)
- `--initial_dimensional_accuracy`: Dimensional accuracy in μm (default: 5.0)
- `--initial_coating_thickness`: Coating thickness in μm (default: 0.0)
- `--initial_stress_level`: Stress level in MPa (default: 50.0)

### Target State (desired final workpiece properties)

- `--target_surface_roughness`: Target surface roughness in μm (default: 2.0)
- `--target_hardness`: Target hardness in HRC (default: 45.0)
- `--target_temperature`: Target temperature in °C (default: 25.0)
- `--target_dimensional_accuracy`: Target dimensional accuracy in μm (default: 0.5)
- `--target_coating_thickness`: Target coating thickness in μm (default: 20.0)
- `--target_stress_level`: Target stress level in MPa (default: 10.0)

### Visualization

- `--plot`: Enable plotting of results and analysis

## Expected Outputs

### Search Results

The system will find Pareto-optimal sequences of machine operations that balance:

1. Processing time
2. Total manufacturing cost
3. Achievement of target specifications

### Example Output Sequence

```
Mode 0*2 Mode 6*1 Mode 3*1 Mode 12*1
```

This represents:

- Milling Machine (Light mode) × 2 operations
- Heat Treatment (Light mode) × 1 operation  
- Polishing Machine (Light mode) × 1 operation
- Coating Station (Standard mode) × 1 operation

### Plots Generated

1. **Pareto Front**: Time vs. Total Cost trade-offs
2. **Cost Breakdown**: Detailed analysis of cost components
3. **Machine Utilization**: Usage statistics across all solutions
4. **State Evolution**: How workpiece properties change over time (simulation mode)

## Technical Implementation

### System Dynamics

Each machine is modeled as a state-space system where:

- **A matrix**: Represents natural evolution and cross-coupling effects
- **B matrix**: Represents how machine inputs affect state variables
- **C matrix**: Output mapping (identity for full state observation)
- **D matrix**: Feedthrough (zero for this system)

### Cost Model

Operation costs are calculated based on:

- Machine efficiency and processing rate
- Energy consumption
- Material usage
- Labor requirements
- Equipment wear
- Quality factors

### Search Strategy

The scheduler synthesis explores different sequences of machine operations to find optimal trade-offs between time, cost, and quality objectives.

This example demonstrates how the FlexMan library can be used to model and optimize complex multi-machine manufacturing systems where each mode represents a fundamentally different piece of equipment with its own characteristics and capabilities.
