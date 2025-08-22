/// @file parameters.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Manufacturing system machine parameters.
///
/// @details
/// This file defines the parameters for different manufacturing machines,
/// each with their specific operational characteristics and costs.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "defines.hpp"
#include <json/json.hpp>
#include <iomanip>
#include <iostream>

namespace manufacturing
{

/// @brief Machine-specific parameters for manufacturing operations.
struct parameters_t {
    // --- Machine identification ---
    MachineType machine_type = MachineType::MillingMachine; ///< Type of machine
    OperationMode operation_mode = OperationMode::Standard; ///< Operation intensity mode
    
    // --- Operational parameters ---
    double primary_param = 100.0;   ///< Primary operation parameter (machine-specific)
    double secondary_param = 50.0;  ///< Secondary operation parameter (machine-specific)
    
    // --- Performance characteristics ---
    double efficiency = 0.85;       ///< [0-1] Machine efficiency factor
    double processing_rate = 1.0;   ///< [units/s] Base processing rate
    double quality_factor = 0.9;    ///< [0-1] Quality factor (higher is better)
    
    // --- Cost parameters ---
    double energy_cost_rate = 0.1;    ///< [$/kWh] Energy cost per unit time
    double material_cost_rate = 0.05; ///< [$/unit] Material consumption rate
    double labor_cost_rate = 25.0;    ///< [$/hour] Labor cost rate
    double wear_cost_rate = 0.02;     ///< [$/unit] Equipment wear cost rate
    
    // --- Machine-specific effectiveness on different state variables ---
    // These represent how much each machine affects each state variable per operation
    double surface_roughness_effect = 0.0;   ///< Effect on surface roughness (μm change)
    double hardness_effect = 0.0;            ///< Effect on hardness (HRC change)
    double temperature_effect = 0.0;         ///< Effect on temperature (°C change)
    double dimensional_accuracy_effect = 0.0; ///< Effect on dimensional accuracy (μm change)
    double coating_thickness_effect = 0.0;   ///< Effect on coating thickness (μm change)
    double stress_level_effect = 0.0;        ///< Effect on stress level (MPa change)
    
    /// @brief Default constructor.
    parameters_t() = default;
    
    /// @brief Constructor for specific machine type and operation mode.
    parameters_t(MachineType machine, OperationMode mode) 
        : machine_type(machine), operation_mode(mode)
    {
        configure_for_machine_type();
        apply_operation_mode_modifiers();
    }

private:
    /// @brief Configure parameters based on machine type.
    void configure_for_machine_type()
    {
        switch (machine_type) {
            case MachineType::MillingMachine:
                primary_param = 1500.0;  // RPM
                secondary_param = 0.5;   // Feed rate (mm/rev)
                efficiency = 0.80;
                processing_rate = 0.8;
                quality_factor = 0.85;
                energy_cost_rate = 0.15;
                material_cost_rate = 0.10;
                labor_cost_rate = 30.0;
                wear_cost_rate = 0.05;
                // Effects on state variables
                surface_roughness_effect = -2.0;    // Improves surface (reduces roughness)
                dimensional_accuracy_effect = -1.0; // Improves accuracy
                temperature_effect = 15.0;          // Increases temperature
                stress_level_effect = 5.0;          // Increases stress slightly
                break;
                
            case MachineType::HeatTreatment:
                primary_param = 850.0;   // Temperature (°C)
                secondary_param = 60.0;  // Duration (minutes)
                efficiency = 0.90;
                processing_rate = 0.3;
                quality_factor = 0.95;
                energy_cost_rate = 0.25;
                material_cost_rate = 0.02;
                labor_cost_rate = 20.0;
                wear_cost_rate = 0.01;
                // Effects on state variables
                hardness_effect = 15.0;        // Significantly increases hardness
                stress_level_effect = -20.0;   // Relieves stress (negative = reduction)
                temperature_effect = 300.0;    // Major temperature increase during process
                break;
                
            case MachineType::CoatingStation:
                primary_param = 250.0;   // Spray pressure (PSI)
                secondary_param = 2.0;   // Application speed (m/min)
                efficiency = 0.75;
                processing_rate = 0.6;
                quality_factor = 0.80;
                energy_cost_rate = 0.08;
                material_cost_rate = 0.20;
                labor_cost_rate = 25.0;
                wear_cost_rate = 0.03;
                // Effects on state variables
                coating_thickness_effect = 25.0;    // Adds coating
                surface_roughness_effect = 1.5;     // Slightly increases roughness
                temperature_effect = 5.0;           // Slight temperature increase
                break;
                
            case MachineType::PolishingMachine:
                primary_param = 3000.0;  // RPM
                secondary_param = 2.5;   // Pressure (kg/cm²)
                efficiency = 0.85;
                processing_rate = 1.2;
                quality_factor = 0.90;
                energy_cost_rate = 0.12;
                material_cost_rate = 0.08;
                labor_cost_rate = 22.0;
                wear_cost_rate = 0.04;
                // Effects on state variables
                surface_roughness_effect = -5.0;    // Significantly improves surface
                coating_thickness_effect = -2.0;    // May remove some coating
                temperature_effect = 8.0;           // Slight temperature increase
                break;
                
            case MachineType::QualityInspection:
                primary_param = 1.0;     // Measurement precision level
                secondary_param = 30.0;  // Inspection time (seconds)
                efficiency = 0.98;
                processing_rate = 2.0;
                quality_factor = 1.0;
                energy_cost_rate = 0.02;
                material_cost_rate = 0.0;
                labor_cost_rate = 35.0;
                wear_cost_rate = 0.001;
                // Effects on state variables (minimal - mainly measurement)
                // QA doesn't change the part, but provides feedback
                break;
                
            case MachineType::CoolingStation:
                primary_param = 5.0;     // Cooling rate (°C/min)
                secondary_param = 25.0;  // Target temperature (°C)
                efficiency = 0.95;
                processing_rate = 0.5;
                quality_factor = 0.85;
                energy_cost_rate = 0.05;
                material_cost_rate = 0.01;
                labor_cost_rate = 15.0;
                wear_cost_rate = 0.005;
                // Effects on state variables
                temperature_effect = -50.0;    // Significantly reduces temperature
                stress_level_effect = -5.0;    // Slight stress relief during cooling
                break;
        }
    }
    
    /// @brief Apply operation mode modifiers to base parameters.
    void apply_operation_mode_modifiers()
    {
        switch (operation_mode) {
            case OperationMode::Light:
                // Faster, cheaper, less effective
                processing_rate *= 1.5;
                energy_cost_rate *= 0.7;
                material_cost_rate *= 0.8;
                labor_cost_rate *= 0.9;
                wear_cost_rate *= 0.6;
                // Reduce all effects by 60%
                surface_roughness_effect *= 0.6;
                hardness_effect *= 0.6;
                temperature_effect *= 0.6;
                dimensional_accuracy_effect *= 0.6;
                coating_thickness_effect *= 0.6;
                stress_level_effect *= 0.6;
                quality_factor *= 0.8;
                break;
                
            case OperationMode::Standard:
                // No modification - base values
                break;
                
            case OperationMode::Intensive:
                // Slower, more expensive, more effective
                processing_rate *= 0.6;
                energy_cost_rate *= 1.4;
                material_cost_rate *= 1.2;
                labor_cost_rate *= 1.1;
                wear_cost_rate *= 1.8;
                // Increase all effects by 50%
                surface_roughness_effect *= 1.5;
                hardness_effect *= 1.5;
                temperature_effect *= 1.5;
                dimensional_accuracy_effect *= 1.5;
                coating_thickness_effect *= 1.5;
                stress_level_effect *= 1.5;
                quality_factor *= 1.15;
                break;
        }
    }
};

/// @brief Stream output operator for parameters.
inline std::ostream &operator<<(std::ostream &lhs, const parameters_t &rhs)
{
    lhs << "Machine: " << static_cast<int>(rhs.machine_type) 
        << ", Mode: " << static_cast<int>(rhs.operation_mode)
        << ", Primary: " << rhs.primary_param 
        << ", Secondary: " << rhs.secondary_param
        << ", Efficiency: " << rhs.efficiency
        << ", Quality: " << rhs.quality_factor;
    return lhs;
}

} // namespace manufacturing

namespace json
{

/// @brief JSON serialization for parameters_t.
inline json::jnode_t &operator<<(json::jnode_t &lhs, const manufacturing::parameters_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["machine_type"] << static_cast<int>(rhs.machine_type);
    lhs["operation_mode"] << static_cast<int>(rhs.operation_mode);
    lhs["primary_param"] << rhs.primary_param;
    lhs["secondary_param"] << rhs.secondary_param;
    lhs["efficiency"] << rhs.efficiency;
    lhs["processing_rate"] << rhs.processing_rate;
    lhs["quality_factor"] << rhs.quality_factor;
    lhs["energy_cost_rate"] << rhs.energy_cost_rate;
    lhs["material_cost_rate"] << rhs.material_cost_rate;
    lhs["labor_cost_rate"] << rhs.labor_cost_rate;
    lhs["wear_cost_rate"] << rhs.wear_cost_rate;
    lhs["surface_roughness_effect"] << rhs.surface_roughness_effect;
    lhs["hardness_effect"] << rhs.hardness_effect;
    lhs["temperature_effect"] << rhs.temperature_effect;
    lhs["dimensional_accuracy_effect"] << rhs.dimensional_accuracy_effect;
    lhs["coating_thickness_effect"] << rhs.coating_thickness_effect;
    lhs["stress_level_effect"] << rhs.stress_level_effect;
    return lhs;
}

/// @brief JSON deserialization for parameters_t.
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, manufacturing::parameters_t &rhs)
{
    int machine_type, operation_mode;
    lhs["machine_type"] >> machine_type;
    lhs["operation_mode"] >> operation_mode;
    rhs.machine_type = static_cast<manufacturing::MachineType>(machine_type);
    rhs.operation_mode = static_cast<manufacturing::OperationMode>(operation_mode);
    lhs["primary_param"] >> rhs.primary_param;
    lhs["secondary_param"] >> rhs.secondary_param;
    lhs["efficiency"] >> rhs.efficiency;
    lhs["processing_rate"] >> rhs.processing_rate;
    lhs["quality_factor"] >> rhs.quality_factor;
    lhs["energy_cost_rate"] >> rhs.energy_cost_rate;
    lhs["material_cost_rate"] >> rhs.material_cost_rate;
    lhs["labor_cost_rate"] >> rhs.labor_cost_rate;
    lhs["wear_cost_rate"] >> rhs.wear_cost_rate;
    lhs["surface_roughness_effect"] >> rhs.surface_roughness_effect;
    lhs["hardness_effect"] >> rhs.hardness_effect;
    lhs["temperature_effect"] >> rhs.temperature_effect;
    lhs["dimensional_accuracy_effect"] >> rhs.dimensional_accuracy_effect;
    lhs["coating_thickness_effect"] >> rhs.coating_thickness_effect;
    lhs["stress_level_effect"] >> rhs.stress_level_effect;
    return lhs;
}

} // namespace json
