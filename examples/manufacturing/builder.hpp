/// @file builder.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Builder for manufacturing system modes.
///
/// @details
/// This file provides the builder for creating manufacturing system modes,
/// where each mode represents a different machine (milling, heat treatment,
/// coating, polishing, quality inspection, cooling) with specific operational
/// characteristics and effects on workpiece properties.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "defines.hpp"
#include "parameters.hpp"

#include <cmath>

namespace manufacturing
{

/// @brief Mode representation for discrete manufacturing systems.
class discrete_mode_t : public flexman::core::Mode<discrete_system_t, input_t>
{
public:
    parameters_t parameters;    ///< Parameters for the discrete mode.
    resources_t operation_cost; ///< Pre-computed operation cost per step
    std::string machine_name;   ///< Human-readable machine name

    /// @brief Default constructor.
    discrete_mode_t() = default;
};

/// @brief Mode representation for continuous manufacturing systems.
class continuous_mode_t : public flexman::core::Mode<continuous_system_t, input_t>
{
public:
    parameters_t parameters;    ///< Parameters for the continuous mode.
    resources_t operation_cost; ///< Pre-computed operation cost per step
    std::string machine_name;   ///< Human-readable machine name

    /// @brief Default constructor.
    continuous_mode_t() = default;
};

/// @brief Builder for manufacturing system modes.
struct builder_t : public parameters_t {
    builder_t(parameters_t params = parameters_t())
        : parameters_t(std::move(params))
    {
        // Nothing to do.
    }

    /// @brief Creates a continuous-time state space model for a manufacturing machine.
    inline auto make_continuous_mode(flexman::core::ModeId id) const noexcept
    {
        continuous_mode_t mode;
        mode.id           = id;
        mode.parameters   = *this;
        mode.machine_name = get_machine_name();

        // Set input parameters (machine-specific)
        mode.input = {primary_param, secondary_param};

        // Pre-compute operation cost for this mode
        mode.operation_cost = calculate_operation_cost();

        // Create the state transition matrix A(6x6)
        // State vector: [surface_roughness, hardness, temperature, dimensional_accuracy, coating_thickness, stress_level]
        // Each machine has different effects on different state variables

        mode.system.A = create_state_transition_matrix();
        mode.system.B = create_input_matrix();
        mode.system.C = create_output_matrix();
        mode.system.D = create_feedthrough_matrix();

        return mode;
    }

    /// @brief Creates a discrete-time mode.
    inline auto make_discrete_mode(flexman::core::ModeId id, double sample_time) const noexcept
    {
        // First, create the continuous-time mode.
        continuous_mode_t ct_mode = this->make_continuous_mode(id);

        // Create the discrete-time mode
        discrete_mode_t mode;
        mode.id             = id;
        mode.input          = ct_mode.input;
        mode.system         = fsmlib::control::c2d(ct_mode.system, sample_time);
        mode.parameters     = *this;
        mode.operation_cost = ct_mode.operation_cost;
        mode.machine_name   = ct_mode.machine_name;

        return mode;
    }

private:
    /// @brief Get human-readable machine name.
    std::string get_machine_name() const
    {
        std::string base_name;
        switch (machine_type) {
        case MachineType::MillingMachine:
            base_name = "Milling Machine";
            break;
        case MachineType::HeatTreatment:
            base_name = "Heat Treatment";
            break;
        case MachineType::CoatingStation:
            base_name = "Coating Station";
            break;
        case MachineType::PolishingMachine:
            base_name = "Polishing Machine";
            break;
        case MachineType::QualityInspection:
            base_name = "Quality Inspection";
            break;
        case MachineType::CoolingStation:
            base_name = "Cooling Station";
            break;
        }

        std::string mode_suffix;
        switch (operation_mode) {
        case OperationMode::Light:
            mode_suffix = " (Light)";
            break;
        case OperationMode::Standard:
            mode_suffix = " (Standard)";
            break;
        case OperationMode::Intensive:
            mode_suffix = " (Intensive)";
            break;
        }

        return base_name + mode_suffix;
    }

    /// @brief Calculate operation cost per time step.
    resources_t calculate_operation_cost() const
    {
        // Base time per operation (adjusted by processing rate)
        double base_time = 1.0 / processing_rate;

        // Calculate costs based on machine operation
        double energy      = energy_cost_rate * base_time * efficiency;
        double energy_cost = energy * 0.15; // Cost per kWh (example rate)
        double material    = material_cost_rate * base_time;
        double labor       = (labor_cost_rate / 3600.0) * base_time; // Convert hourly rate to per-second
        double wear        = wear_cost_rate * base_time;
        double quality     = (1.0 - quality_factor) * 10.0; // Quality loss penalty

        return resources_t(base_time, energy, energy_cost, material, labor, wear, quality);
    }

    /// @brief Create state transition matrix A(6x6).
    fsmlib::Matrix<double, n_states, n_states> create_state_transition_matrix() const
    {
        // Identity matrix with small decay factors to represent natural changes
        fsmlib::Matrix<double, n_states, n_states> A;

        // Initialize as identity matrix with slight decay
        for (std::size_t i = 0; i < n_states; ++i) {
            for (std::size_t j = 0; j < n_states; ++j) {
                if (i == j) {
                    A(i, j) = 0.99; // Slight decay over time
                } else {
                    A(i, j) = 0.0;
                }
            }
        }

        // Add machine-specific cross-effects (how one property affects another)
        switch (machine_type) {
        case MachineType::MillingMachine:
            // Milling can affect surface-dimensional accuracy coupling
            A(0, 3) = 0.05; // surface roughness affects dimensional accuracy
            A(2, 5) = 0.02; // temperature affects stress
            break;

        case MachineType::HeatTreatment:
            // Heat treatment affects multiple properties
            A(1, 5) = -0.1; // hardness reduces stress
            A(2, 1) = 0.03; // temperature affects hardness
            A(2, 5) = 0.05; // temperature affects stress
            break;

        case MachineType::CoatingStation:
            // Coating affects surface properties
            A(4, 0) = 0.02; // coating thickness affects surface roughness
            break;

        case MachineType::PolishingMachine:
            // Polishing creates coupling between surface and dimensional properties
            A(0, 3) = -0.02; // better surface improves dimensional accuracy
            break;

        case MachineType::QualityInspection:
            // QA doesn't change properties, just measures them
            break;

        case MachineType::CoolingStation:
            // Cooling affects thermal and mechanical properties
            A(2, 5) = -0.03; // cooling reduces stress
            break;
        }

        return A;
    }

    /// @brief Create input matrix B(6x2).
    fsmlib::Matrix<double, n_states, n_input> create_input_matrix() const
    {
        fsmlib::Matrix<double, n_states, n_input> B;

        // Initialize all elements to zero
        for (std::size_t i = 0; i < n_states; ++i) {
            for (std::size_t j = 0; j < n_input; ++j) {
                B(i, j) = 0.0;
            }
        }

        // Define how inputs affect state variables based on machine type
        // Input 0: primary parameter, Input 1: secondary parameter
        switch (machine_type) {
        case MachineType::MillingMachine:
            // Primary: RPM affects surface and temperature
            B(0, 0) = surface_roughness_effect * 0.001 * efficiency;     // Surface roughness
            B(2, 0) = temperature_effect * 0.0001 * efficiency;          // Temperature
            B(3, 0) = dimensional_accuracy_effect * 0.0005 * efficiency; // Dimensional accuracy
            B(5, 0) = stress_level_effect * 0.0002 * efficiency;         // Stress
            // Secondary: Feed rate affects quality and wear
            B(0, 1) = surface_roughness_effect * 0.1 * efficiency;
            B(3, 1) = dimensional_accuracy_effect * 0.05 * efficiency;
            break;

        case MachineType::HeatTreatment:
            // Primary: Temperature significantly affects hardness and stress
            B(1, 0) = hardness_effect * 0.01 * efficiency;     // Hardness
            B(2, 0) = temperature_effect * 0.1 * efficiency;   // Temperature
            B(5, 0) = stress_level_effect * 0.01 * efficiency; // Stress
            // Secondary: Duration affects depth of treatment
            B(1, 1) = hardness_effect * 0.001 * efficiency;
            B(5, 1) = stress_level_effect * 0.002 * efficiency;
            break;

        case MachineType::CoatingStation:
            // Primary: Pressure affects coating thickness and surface
            B(0, 0) = surface_roughness_effect * 0.001 * efficiency; // Surface
            B(4, 0) = coating_thickness_effect * 0.01 * efficiency;  // Coating thickness
            // Secondary: Speed affects uniformity
            B(4, 1) = coating_thickness_effect * 0.001 * efficiency;
            break;

        case MachineType::PolishingMachine:
            // Primary: RPM affects surface finish
            B(0, 0) = surface_roughness_effect * 0.001 * efficiency;  // Surface roughness
            B(2, 0) = temperature_effect * 0.0001 * efficiency;       // Temperature
            B(4, 0) = coating_thickness_effect * 0.0005 * efficiency; // May remove coating
            // Secondary: Pressure affects effectiveness
            B(0, 1) = surface_roughness_effect * 0.01 * efficiency;
            break;

        case MachineType::QualityInspection:
            // QA doesn't directly change the workpiece
            // But we can model measurement feedback effects minimally
            break;

        case MachineType::CoolingStation:
            // Primary: Cooling rate affects temperature and stress
            B(2, 0) = temperature_effect * 0.1 * efficiency;   // Temperature
            B(5, 0) = stress_level_effect * 0.01 * efficiency; // Stress
            // Secondary: Target temperature affects final state
            B(2, 1) = -0.001 * efficiency; // Drives toward target
            break;
        }

        return B;
    }

    /// @brief Create output matrix C(6x6) - identity matrix to observe all states.
    fsmlib::Matrix<double, n_output, n_states> create_output_matrix() const
    {
        fsmlib::Matrix<double, n_output, n_states> C;

        // Identity matrix - we can observe all state variables
        for (std::size_t i = 0; i < n_output && i < n_states; ++i) {
            for (std::size_t j = 0; j < n_states; ++j) {
                C(i, j) = (i == j) ? 1.0 : 0.0;
            }
        }

        return C;
    }

    /// @brief Create feedthrough matrix D(6x2) - zero matrix.
    fsmlib::Matrix<double, n_output, n_input> create_feedthrough_matrix() const
    {
        fsmlib::Matrix<double, n_output, n_input> D;

        // Zero matrix - no direct feedthrough from input to output
        for (std::size_t i = 0; i < n_output; ++i) {
            for (std::size_t j = 0; j < n_input; ++j) {
                D(i, j) = 0.0;
            }
        }

        return D;
    }
};

/// @brief Stream output operator for discrete mode.
inline std::ostream &operator<<(std::ostream &lhs, const discrete_mode_t &rhs)
{
    lhs << "Mode ID: " << rhs.id << " - " << rhs.machine_name << "\n";
    lhs << "System:\n"
        << rhs.system << "\n";
    lhs << "Input: " << rhs.input << "\n";
    lhs << "Operation Cost: " << rhs.operation_cost << "\n";
    lhs << "Parameters: " << rhs.parameters << "\n";
    return lhs;
}

/// @brief Stream output operator for continuous mode.
inline std::ostream &operator<<(std::ostream &lhs, const continuous_mode_t &rhs)
{
    lhs << "Mode ID: " << rhs.id << " - " << rhs.machine_name << "\n";
    lhs << "System:\n"
        << rhs.system << "\n";
    lhs << "Input: " << rhs.input << "\n";
    lhs << "Operation Cost: " << rhs.operation_cost << "\n";
    lhs << "Parameters: " << rhs.parameters << "\n";
    return lhs;
}

} // namespace manufacturing

namespace json
{

/// @brief JSON serialization for discrete_mode_t.
inline json::jnode_t &operator<<(json::jnode_t &lhs, const manufacturing::discrete_mode_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["id"] << rhs.id;
    lhs["system"] << rhs.system;
    lhs["input"] << rhs.input;
    lhs["parameters"] << rhs.parameters;
    lhs["operation_cost"] << rhs.operation_cost;
    lhs["machine_name"] << rhs.machine_name;
    return lhs;
}

/// @brief JSON deserialization for discrete_mode_t.
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, manufacturing::discrete_mode_t &rhs)
{
    lhs["id"] >> rhs.id;
    lhs["system"] >> rhs.system;
    lhs["input"] >> rhs.input;
    lhs["parameters"] >> rhs.parameters;
    lhs["operation_cost"] >> rhs.operation_cost;
    lhs["machine_name"] >> rhs.machine_name;
    return lhs;
}

/// @brief JSON serialization for continuous_mode_t.
inline json::jnode_t &operator<<(json::jnode_t &lhs, const manufacturing::continuous_mode_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["id"] << rhs.id;
    lhs["system"] << rhs.system;
    lhs["input"] << rhs.input;
    lhs["parameters"] << rhs.parameters;
    lhs["operation_cost"] << rhs.operation_cost;
    lhs["machine_name"] << rhs.machine_name;
    return lhs;
}

/// @brief JSON deserialization for continuous_mode_t.
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, manufacturing::continuous_mode_t &rhs)
{
    lhs["id"] >> rhs.id;
    lhs["system"] >> rhs.system;
    lhs["input"] >> rhs.input;
    lhs["parameters"] >> rhs.parameters;
    lhs["operation_cost"] >> rhs.operation_cost;
    lhs["machine_name"] >> rhs.machine_name;
    return lhs;
}

} // namespace json
