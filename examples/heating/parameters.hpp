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

/// @brief Parameters.
struct parameters_t {
    double mass = 1.0;     // [kg] mass of the metal workpiece
    double area = 0.1;     // [m^2] surface area
    double eta = 0.8;      // efficiency factor [0, 1]
    double T_env = 20.0;   // [C] ambient temperature
    double h = 10.0;       // [W/m^2*C] heat transfer coefficient
    double T_init = 20.0;  // [C] initial temperature
    double Cp0 = 500.0;    // [J/kg*C] heat capacity at reference temperature
    double Cp_slope = 0.1; // [J/kg*C^2] temperature dependence
    double R0 = 0.01;      // [Ohm] resistance at reference temperature
    double R_slope = 0.0001; // [Ohm/C] temperature dependence
};

inline std::ostream &operator<<(std::ostream &lhs, const parameters_t &rhs)
{
    lhs << "[" << rhs.mass << ", " << rhs.area << ", " << rhs.eta << ", " << rhs.T_env << ", " << rhs.h << ", "
        << rhs.T_init << ", " << rhs.Cp0 << ", " << rhs.Cp_slope << ", " << rhs.R0 << ", " << rhs.R_slope << "]";
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
    lhs["eta"] << rhs.eta;
    lhs["T_env"] << rhs.T_env;
    lhs["h"] << rhs.h;
    lhs["T_init"] << rhs.T_init;
    lhs["Cp0"] << rhs.Cp0;
    lhs["Cp_slope"] << rhs.Cp_slope;
    lhs["R0"] << rhs.R0;
    lhs["R_slope"] << rhs.R_slope;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, heating::parameters_t &rhs)
{
    lhs["mass"] >> rhs.mass;
    lhs["area"] >> rhs.area;
    lhs["eta"] >> rhs.eta;
    lhs["T_env"] >> rhs.T_env;
    lhs["h"] >> rhs.h;
    lhs["T_init"] >> rhs.T_init;
    lhs["Cp0"] >> rhs.Cp0;
    lhs["Cp_slope"] >> rhs.Cp_slope;
    lhs["R0"] >> rhs.R0;
    lhs["R_slope"] >> rhs.R_slope;
    return lhs;
}

} // namespace json
