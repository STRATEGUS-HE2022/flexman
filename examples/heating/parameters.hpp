/// @file parameters.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines the parameters.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#include <json/json.hpp>

namespace heating
{

/// @brief Physical and model parameters for the thermal system.
struct parameters_t {
    // --- Workpiece properties ---
    double mass = 2.32;  ///< [kg] mass of the workpiece
    double area = 0.5;   ///< [m^2] surface area exposed for heat transfer
    double Cp0  = 400.0; ///< [J/kg*C] heat capacity of workpiece at reference temperature

    // --- Heater/electrical properties ---
    double eta = 0.8; ///< [–] efficiency factor [0,1]

    // --- Environment node (second state) ---
    double env_mass = 1.0;   ///< [kg] effective thermal mass of environment
    double Cp_env   = 800.0; ///< [J/kg*C] environment heat capacity

    // --- Coupling parameters ---
    double h            = 10.0; ///< [W/m^2*C] heat transfer coefficient
    double G_leak0      = 1.0;  ///< [W/°C] baseline leak at zero power
    double k_leak_per_w = 0.01; ///< [W/°C per W] leak gain per watt of heater power
    double G_leak_min   = 0.5;  ///< [W/°C] minimum leak conductance
    double G_leak_max   = 6.0;  ///< [W/°C] maximum leak conductance
    
    // --- Fan power parameters ---
    double fan_P0       = 8.0;  ///< [W] idle fan draw at baseline leak
    double fan_P_cubic  = 0.5;  ///< [W] scale factor for cubic fan power term

    // --- Setting parameters ---
    double input_power = 0.0; ///< [W] The input power.
};

inline std::ostream &operator<<(std::ostream &lhs, const parameters_t &rhs)
{
    lhs << "mass: " << rhs.mass << " kg, "
        << "area: " << rhs.area << " m^2, "
        << "Cp0: " << rhs.Cp0 << " J/kg*C, "
        << "eta: " << rhs.eta << ", "
        << "env_mass: " << rhs.env_mass << " kg, "
        << "Cp_env: " << rhs.Cp_env << " J/kg*C, "
        << "h: " << rhs.h << " W/m^2*C, "
        << "G_leak0: " << rhs.G_leak0 << " W/C, "
        << "k_leak_per_w: " << rhs.k_leak_per_w << " (W/°C) per W, "
        << "G_leak_min: " << rhs.G_leak_min << " W/°C, "
        << "G_leak_max: " << rhs.G_leak_max << " W/°C, "
        << "input_power: " << rhs.input_power << " W\n";
    return lhs;
}

} // namespace heating

namespace json
{

inline json::jnode_t &operator<<(json::jnode_t &lhs, const heating::parameters_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["mass"] << rhs.mass;
    lhs["area"] << rhs.area;
    lhs["Cp0"] << rhs.Cp0;
    lhs["eta"] << rhs.eta;
    lhs["env_mass"] << rhs.env_mass;
    lhs["Cp_env"] << rhs.Cp_env;
    lhs["h"] << rhs.h;
    lhs["G_leak0"] << rhs.G_leak0;
    lhs["k_leak_per_w"] << rhs.k_leak_per_w;
    lhs["G_leak_min"] << rhs.G_leak_min;
    lhs["G_leak_max"] << rhs.G_leak_max;
    lhs["input_power"] << rhs.input_power;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, heating::parameters_t &rhs)
{
    lhs["mass"] >> rhs.mass;
    lhs["area"] >> rhs.area;
    lhs["Cp0"] >> rhs.Cp0;
    lhs["eta"] >> rhs.eta;
    lhs["env_mass"] >> rhs.env_mass;
    lhs["Cp_env"] >> rhs.Cp_env;
    lhs["h"] >> rhs.h;
    lhs["G_leak0"] >> rhs.G_leak0;
    lhs["k_leak_per_w"] >> rhs.k_leak_per_w;
    lhs["G_leak_min"] >> rhs.G_leak_min;
    lhs["G_leak_max"] >> rhs.G_leak_max;
    lhs["input_power"] >> rhs.input_power;
    return lhs;
}

} // namespace json
