/// @file parameters.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines the parameters for the tapping system.
///
/// @details
/// This file provides the `parameters_t` structure, which encapsulates
/// key physical and operational parameters of the tapping system.
/// It includes:
/// - Electrical parameters (voltage, resistance, inductance).
/// - Mechanical properties (moment of inertia, friction coefficients).
/// - System constants (gear ratio, thread slope, switching cost and time).
///
/// Additionally, this file provides:
/// - Overloaded stream operators for easy output of parameter values.
/// - JSON serialization and deserialization operators for storing and
///   loading parameter configurations.
///
/// These definitions enable structured handling of tapping system
/// parameters for modeling, simulation, and optimization processes.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#include <json/json.hpp>

namespace tapping
{

/// @brief Tapping parameters.
struct parameters_t {
    double V;  ///< Supplied voltage[V].
    double R;  ///< Winding resistance in Ohms.
    double L;  ///< Winding inductance in Henrys[H].
    double J;  ///< Angular momentum[kg.m ^ 2].
    double Kb; ///< Coulomb friction[N.m].
    double Ke; ///< Back - EMF contanst[V * s / rad].
    double Kt; ///< Torque constant[N * m / A].
    double Fd; ///< Dynamic hole friction[Nm / mm]
    double Fs; ///< Static hole  friction[Nm]
    double Ts; ///< Thread slope, i.e., y - axis depth per revolution[mm / rev].
    double Gr; ///< Gear ratio.
    double Sc; ///< switch cost
    double St; ///< switch time

    parameters_t()
        : V(48.0)
        , // Common industrial machine voltage
        R(1.2)
        , // Typical motor winding resistance
        L(50e-05)
        , // Typical winding inductance
        J(0.2)
        , // Slightly higher angular momentum for industrial loads
        Kb(0.5)
        , // Increased Coulomb friction
        Ke(1.1)
        , // Adjusted Back-EMF constant
        Kt(1.2)
        , // Stronger torque per ampere
        Fd(0.02)
        , // Slightly higher dynamic friction
        Fs(0.15)
        , // Higher static friction
        Ts(1.5)
        , // Larger thread slope for industrial tapping
        Gr(30)
        , // Higher gear ratio for precision
        Sc(0.05)
        ,       // Small but nonzero switch cost
        St(0.2) // Nonzero switch time for industrial processes
    {
        // Nothing to do.
    }
};

inline std::ostream &operator<<(std::ostream &lhs, const parameters_t &rhs)
{
    lhs << "[" << rhs.V << ", " << rhs.R << ", " << rhs.L << ", " << rhs.J << ", " << rhs.Kb << ", " << rhs.Ke << ", "
        << rhs.Kt << ", " << rhs.Fd << ", " << rhs.Fs << ", " << rhs.Ts << ", " << rhs.Gr << ", " << rhs.Sc << ", "
        << rhs.St << "]";
    return lhs;
}

} // namespace tapping

namespace json
{

inline json::jnode_t &operator<<(json::jnode_t &lhs, const tapping::parameters_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["V"] << rhs.V;
    lhs["R"] << rhs.R;
    lhs["L"] << rhs.L;
    lhs["J"] << rhs.J;
    lhs["Kb"] << rhs.Kb;
    lhs["Ke"] << rhs.Ke;
    lhs["Kt"] << rhs.Kt;
    lhs["Fd"] << rhs.Fd;
    lhs["Fs"] << rhs.Fs;
    lhs["Ts"] << rhs.Ts;
    lhs["Gr"] << rhs.Gr;
    lhs["Sc"] << rhs.Sc;
    lhs["St"] << rhs.St;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, tapping::parameters_t &rhs)
{
    lhs["V"] >> rhs.V;
    lhs["R"] >> rhs.R;
    lhs["L"] >> rhs.L;
    lhs["J"] >> rhs.J;
    lhs["Kb"] >> rhs.Kb;
    lhs["Ke"] >> rhs.Ke;
    lhs["Kt"] >> rhs.Kt;
    lhs["Fd"] >> rhs.Fd;
    lhs["Fs"] >> rhs.Fs;
    lhs["Ts"] >> rhs.Ts;
    lhs["Gr"] >> rhs.Gr;
    lhs["Sc"] >> rhs.Sc;
    lhs["St"] >> rhs.St;
    return lhs;
}

} // namespace json
