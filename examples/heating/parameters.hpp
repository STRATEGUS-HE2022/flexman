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
    double mass     = 1.0;   ///< [kg] mass of the workpiece
    double area     = 0.1;   ///< [m^2] surface area exposed for heat transfer
    double Cp0      = 1000.0; ///< [J/kg*C] heat capacity of workpiece at reference temperature
    double T_init   = 20.0;  ///< [C] initial workpiece temperature

    // --- Heater/electrical properties ---
    double eta     = 0.8;    ///< [–] efficiency factor [0,1]
    double R0      = 0.01;   ///< [Ohm] resistance at reference temperature

    // --- Environment node (second state) ---
    double env_mass   = 1.0;    ///< [kg] effective thermal mass of environment
    double Cp_env     = 1000.0; ///< [J/kg*C] environment heat capacity
    double T_env_init = 20.0;   ///< [C] initial environment temperature

    // --- Coupling parameters ---
    double h      = 10.0; ///< [W/m^2*C] heat transfer coefficient
    double G_leak = 0.0;  ///< [W/C] leak conductance from environment to far field (0 = closed system)
};

inline std::ostream &operator<<(std::ostream &lhs, const parameters_t &rhs)
{
    lhs << "Parameters:\n"
        << "  mass: " << rhs.mass << " kg\n"
        << "  area: " << rhs.area << " m^2\n"
        << "  Cp0: " << rhs.Cp0 << " J/kg*C\n"
        << "  T_init: " << rhs.T_init << " C\n"
        << "  eta: " << rhs.eta << "\n"
        << "  R0: " << rhs.R0 << " Ohm\n"
        << "  env_mass: " << rhs.env_mass << " kg\n"
        << "  Cp_env: " << rhs.Cp_env << " J/kg*C\n"
        << "  T_env_init: " << rhs.T_env_init << " C\n"
        << "  h: " << rhs.h << " W/m^2*C\n"
        << "  G_leak: " << rhs.G_leak << " W/C\n";
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
    lhs["T_init"] << rhs.T_init;
    lhs["eta"] << rhs.eta;
    lhs["R0"] << rhs.R0;
    lhs["env_mass"] << rhs.env_mass;
    lhs["Cp_env"] << rhs.Cp_env;
    lhs["T_env_init"] << rhs.T_env_init;
    lhs["h"] << rhs.h;
    lhs["G_leak"] << rhs.G_leak;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, heating::parameters_t &rhs)
{
    lhs["mass"] >> rhs.mass;
    lhs["area"] >> rhs.area;
    lhs["Cp0"] >> rhs.Cp0;
    lhs["T_init"] >> rhs.T_init;
    lhs["eta"] >> rhs.eta;
    lhs["R0"] >> rhs.R0;
    lhs["env_mass"] >> rhs.env_mass;
    lhs["Cp_env"] >> rhs.Cp_env;
    lhs["T_env_init"] >> rhs.T_env_init;
    lhs["h"] >> rhs.h;
    lhs["G_leak"] >> rhs.G_leak;
    return lhs;
}

} // namespace json
