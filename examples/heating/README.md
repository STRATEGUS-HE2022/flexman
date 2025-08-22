# Heating System Model Documentation

This document provides comprehensive documentation for the induction heating system model used in the Flexman framework. The system models the thermal dynamics of metal workpiece heating with power-dependent losses and fan power consumption.

## Table of Contents

1. [System Overview](#system-overview)
2. [Physical Model](#physical-model)
3. [Mathematical Formulation](#mathematical-formulation)
4. [Fan Power Model](#fan-power-model)
5. [ABCD System Implementation](#abcd-system-implementation)
6. [Energy Accounting](#energy-accounting)
7. [Parameters Reference](#parameters-reference)
8. [Implementation Notes](#implementation-notes)
9. [Examples and Validation](#examples-and-validation)

## System Overview

The heating system models **induction heating of metal workpieces** using a two-state thermal network:

- **State 1**: Workpiece temperature `T` [°C]
- **State 2**: Environment temperature `T_env` [°C]
- **Input**: Electrical heater power `u` [W]
- **Output**: Workpiece temperature `T` [°C]

### Key Features

- ✅ **Power-dependent heat losses**: Higher power → more airflow → higher convective losses
- ✅ **Cubic fan power scaling**: Realistic fan power consumption based on airflow requirements
- ✅ **Bounded leak conductance**: Physical limits prevent unrealistic behavior
- ✅ **Pre-computed system matrices**: Optimized for fixed input power per mode
- ✅ **Physically realistic parameters**: Based on industrial heating systems

## Physical Model

### Thermal Network

```text
Room (T_inf = const)
    ↑ G_leak(u) [variable]
Environment Node (T_env, C_env)
    ↕ G [fixed]
Workpiece (T, C_workpiece) ← η·u [heater input]
```

### Heat Flow Equations

**Workpiece energy balance:**

```text
C_workpiece · dT/dt = η·u - G·(T - T_env)
```

**Environment energy balance:**

```text
C_env · dT_env/dt = G·(T - T_env) - G_leak(u)·(T_env - T_inf)
```

Where:

- `C_workpiece = mass × Cp0` [J/°C] - Workpiece thermal capacitance
- `C_env = env_mass × Cp_env` [J/°C] - Environment thermal capacitance  
- `G = h × area` [W/°C] - Fixed heat transfer conductance (workpiece ↔ environment)
- `G_leak(u)` [W/°C] - Power-dependent leak conductance (environment → room)
- `η` [-] - Heater efficiency factor
- `T_inf` [°C] - Room temperature (assumed constant)

## Mathematical Formulation

### State-Space Representation

**States**: `x = [T, T_env]ᵀ` [°C]

**Input**: `u` = electrical heater power [W]

**System matrices**:

```text
A = [[-G/Cx,           +G/Cx          ],
     [+G/Ce,    -(G+G_leak_eff)/Ce   ]]

B = [[η/Cx],
     [0.0 ]]

C = [[1.0, 0.0]]  // Output workpiece temperature only

D = [[0.0]]
```

Where:

- `Cx = mass × Cp0` [J/°C]
- `Ce = env_mass × Cp_env` [J/°C]
- `G_leak_eff = clamp(G0 + k_leak × u, G_min, G_max)` [W/°C]

### Power-Dependent Leak Model

The environment-to-room leak conductance varies with heater power to model increased airflow:

```text
G_leak(u) = clamp(G0 + k_leak × u, G_min, G_max)
```

**Parameters:**

- `G0` [W/°C] - Baseline leak at zero power
- `k_leak` [W/°C per W] - Leak gain per watt of heater power
- `G_min, G_max` [W/°C] - Safety bounds

**Physical interpretation**: Higher heater power often requires more cooling airflow (fans, forced convection), leading to higher convective heat losses.

## Fan Power Model

### Motivation

Industrial heating systems use fans/blowers for:

1. **Cooling**: Prevent overheating of electrical components
2. **Airflow**: Maintain proper combustion or heat distribution
3. **Safety**: Remove hazardous gases or maintain pressure

Fan power consumption should increase with heating power demand, creating a trade-off between heating speed and electrical efficiency.

### Mathematical Model

**Fan electrical power**:

```text
P_fan(u) = P_f0 + P_f_cubic × (G_leak(u)/G0)³
```

**Physical basis:**

- **Cubic scaling**: Fan power ∝ (airflow)³ follows standard affinity laws for centrifugal fans
- **Leak-dependent**: More leak conductance correlates with higher airflow requirements
- **Baseline power**: `P_f0` represents idle fan draw for basic cooling

### Parameter Selection

**Recommended values:**

- `P_f0 = 8.0` W - Idle fan draw
- `P_f_cubic = 0.5` W - Cubic scaling factor

**Validation example** (500W heater):

- `G_leak = min(1.0 + 0.01×500, 6.0) = 6.0` W/°C
- `P_fan = 8.0 + 0.5×(6.0/1.0)³ = 8.0 + 108 = 116` W
- **Total electrical**: 500 + 116 = 616 W
- **Fan overhead**: 23% (reasonable for industrial systems)

⚠️ **Previous issue**: `P_f_cubic = 8.0` led to unrealistic fan power (>1000W for medium heater power)

## ABCD System Implementation

### Pre-Computation Strategy

Since input power `u` is **fixed per mode**, all power-dependent calculations can be pre-computed during mode creation:

```cpp
// In builder.hpp - make_continuous_mode()
const double u = input_power;  // Fixed for this mode
const double G_leak_eff = std::clamp(G0 + k_leak * u, Gmin, Gmax);
const double P_fan = Pf0 + Pf_cubic * pow(G_leak_eff/G0, 3);

// Store total electrical power
mode.total_electrical_power = u + P_fan;

// Build system matrices with effective leak
mode.system.A = compute_A_matrix(G_leak_eff);
```

### Benefits

✅ **Performance**: No runtime calculations during simulation  
✅ **Accuracy**: Each mode has exact physics for its power level  
✅ **Maintainability**: All mode-specific physics centralized in builder  
✅ **Consistency**: Same approach for discrete and continuous modes  

## Energy Accounting

### Electrical Energy Consumption

**Per simulation step**:

```text
E_electrical += mode.total_electrical_power × Δt
```

**Components**:

- Heater power: `u` [W]
- Fan power: `P_fan(u)` [W]
- **Total**: `u + P_fan(u)` [W]

### Thermal Energy

**Heat delivered to workpiece**:

```text
Q_delivered = η × u × Δt  [J]
```

**Heat leaked to room**:

```text
Q_leaked = G_leak(u) × (T_env - T_inf) × Δt  [J]
```

**Important**: Only electrical energy is counted in the resource optimization. Thermal energy is a consequence, not a separate cost.

## Parameters Reference

Workpiece Properties:

```cpp
double mass = 2.32;   // [kg] workpiece mass
double area = 0.5;    // [m²] heat transfer surface area
double Cp0 = 400.0;   // [J/kg·°C] heat capacity at reference temperature
```

Heater Properties:

```cpp
double eta = 0.8;     // [-] heater efficiency (0-1)
```

Environment Node:

```cpp
double env_mass = 1.0;    // [kg] effective thermal mass
double Cp_env = 800.0;    // [J/kg·°C] environment heat capacity
```

Heat Transfer:

```cpp
double h = 10.0;          // [W/m²·°C] heat transfer coefficient
```

Leak Conductance Model:

```cpp
double G_leak0 = 1.0;     // [W/°C] baseline leak at zero power
double k_leak_per_w = 0.01; // [W/°C per W] leak gain per watt
double G_leak_min = 0.5;  // [W/°C] minimum leak conductance
double G_leak_max = 6.0;  // [W/°C] maximum leak conductance
```

Fan Power Model:

```cpp
double fan_P0 = 8.0;      // [W] idle fan draw at baseline leak
double fan_P_cubic = 0.5; // [W] cubic scaling factor
```

## Implementation Notes

### Builder Architecture

**Files**:

- `parameters.hpp` - Parameter definitions and defaults
- `builder.hpp` - System matrix construction and mode creation
- `defines.hpp` - Type definitions and constants
- `resources.hpp` - Resource tracking (energy, time)

**Key classes**:

- `parameters_t` - Contains all physical parameters
- `builder_t` - Inherits from `parameters_t`, builds modes
- `continuous_mode_t` - Continuous-time system representation
- `discrete_mode_t` - Discrete-time system representation

### Search Managers

**Files**:

- `search.hpp` - Discrete and continuous search managers

**Key features**:

- Use pre-computed `mode.total_electrical_power`
- No runtime `leak_and_fan()` calculations
- Consistent energy accounting between discrete/continuous

### Mode Creation Process

1. **Set parameters**: Including `input_power` for the mode
2. **Compute physics**: `G_leak_eff`, `P_fan` based on `input_power`
3. **Build matrices**: System matrices A, B, C, D with effective values
4. **Store totals**: `total_electrical_power = u + P_fan`
5. **Return mode**: Ready for simulation with pre-computed values

## Examples and Validation

### Steady-State Analysis

For constant input power `u`, the steady-state temperature rise is approximately:

```
ΔT_steady ≈ (η × u) / (G + G_leak(u))
```

This shows **diminishing returns** as power increases:

- Numerator grows linearly with `u`
- Denominator grows with `u` due to `G_leak(u) = G0 + k_leak × u`
- Result: Temperature saturates, while electrical cost (including fan) continues growing

### Example Calculations

**Low power (100W)**:

- G_leak = 1.0 + 0.01×100 = 2.0 W/°C
- P_fan = 8.0 + 0.5×(2.0)³ = 12.0 W
- Total electrical = 112 W
- ΔT_steady ≈ (0.8×100)/(5.0+2.0) ≈ 11.4°C

**High power (400W)**:

- G_leak = 1.0 + 0.01×400 = 5.0 W/°C  
- P_fan = 8.0 + 0.5×(5.0)³ = 70.5 W
- Total electrical = 470.5 W
- ΔT_steady ≈ (0.8×400)/(5.0+5.0) ≈ 32.0°C

**Efficiency comparison**:

- Low power: 11.4°C / 112W = 0.102 °C/W
- High power: 32.0°C / 470.5W = 0.068 °C/W

This demonstrates the **diminishing returns** that drive the optimization to find optimal power sequences rather than always using maximum power.

### Physical Validation

The model captures realistic industrial heating behavior:

- ✅ Higher power increases both heating rate and losses
- ✅ Fan power grows super-linearly with heating power
- ✅ Optimal power sequences balance time vs. energy consumption
- ✅ System is stable (negative real eigenvalues of A-matrix)
- ✅ Energy conservation is maintained

## Future Extensions

Potential model enhancements:

1. **Temperature-dependent properties**: Variable Cp, thermal conductivity
2. **Radiation losses**: T⁴ dependence for high-temperature applications  
3. **Multi-zone heating**: Spatial temperature distribution
4. **Dynamic fan control**: Separate fan power optimization
5. **Wear models**: Equipment degradation with usage

---

**Authors**: Enrico Fraccaroli, GitHub Copilot  
**Date**: August 2025  
**Version**: 1.0
