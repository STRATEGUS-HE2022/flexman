/// @file resources.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Manufacturing system resource definitions.
///
/// @details
/// This file defines the resources used in the manufacturing system,
/// including time, energy, material costs, and quality metrics.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include <json/json.hpp>
#include <ostream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace manufacturing
{

/// @brief Resources representing manufacturing costs and metrics.
struct resources_t {
    double time = 0.0;           ///< [seconds] Total processing time
    double energy = 0.0;         ///< [kWh] Total energy consumption (required by PSO)
    double energy_cost = 0.0;    ///< [monetary units] Energy consumption cost
    double material_cost = 0.0;  ///< [monetary units] Material and consumables cost
    double labor_cost = 0.0;     ///< [monetary units] Labor cost
    double equipment_wear = 0.0; ///< [units] Equipment wear and maintenance cost
    double quality_loss = 0.0;   ///< [units] Quality degradation penalty

    /// @brief Default constructor.
    resources_t() = default;

    /// @brief Constructor with all parameters.
    resources_t(double _time, double _energy, double _energy_cost, double _material_cost, 
                double _labor_cost, double _equipment_wear, double _quality_loss)
        : time(_time)
        , energy(_energy)
        , energy_cost(_energy_cost)
        , material_cost(_material_cost)
        , labor_cost(_labor_cost)
        , equipment_wear(_equipment_wear)
        , quality_loss(_quality_loss)
    {
        // Nothing to do.
    }

    /// @brief Addition operator.
    resources_t operator+(const resources_t &other) const
    {
        return resources_t(
            time + other.time,
            energy + other.energy,
            energy_cost + other.energy_cost,
            material_cost + other.material_cost,
            labor_cost + other.labor_cost,
            equipment_wear + other.equipment_wear,
            quality_loss + other.quality_loss
        );
    }

    /// @brief Scalar multiplication operator.
    resources_t operator*(double scalar) const
    {
        return resources_t(
            time * scalar,
            energy * scalar,
            energy_cost * scalar,
            material_cost * scalar,
            labor_cost * scalar,
            equipment_wear * scalar,
            quality_loss * scalar
        );
    }

    /// @brief Total cost calculation (excluding time).
    double total_cost() const
    {
        return energy_cost + material_cost + labor_cost + equipment_wear + quality_loss;
    }

    /// @brief String representation.
    std::string to_string() const
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "Time: " << time << "s, ";
        oss << "Total Cost: " << total_cost() << " ";
        oss << "(Energy: " << energy_cost << ", ";
        oss << "Material: " << material_cost << ", ";
        oss << "Labor: " << labor_cost << ", ";
        oss << "Wear: " << equipment_wear << ", ";
        oss << "Quality: " << quality_loss << ")";
        return oss.str();
    }
};

/// @brief Addition assignment operator.
inline resources_t &operator+=(resources_t &lhs, const resources_t &rhs) noexcept
{
    lhs.time += rhs.time;
    lhs.energy += rhs.energy;
    lhs.energy_cost += rhs.energy_cost;
    lhs.material_cost += rhs.material_cost;
    lhs.labor_cost += rhs.labor_cost;
    lhs.equipment_wear += rhs.equipment_wear;
    lhs.quality_loss += rhs.quality_loss;
    return lhs;
}

/// @brief Equality operator.
inline bool operator==(const resources_t &lhs, const resources_t &rhs) noexcept
{
    const double eps = 1e-9;
    return std::abs(lhs.time - rhs.time) < eps &&
           std::abs(lhs.energy - rhs.energy) < eps &&
           std::abs(lhs.energy_cost - rhs.energy_cost) < eps &&
           std::abs(lhs.material_cost - rhs.material_cost) < eps &&
           std::abs(lhs.labor_cost - rhs.labor_cost) < eps &&
           std::abs(lhs.equipment_wear - rhs.equipment_wear) < eps &&
           std::abs(lhs.quality_loss - rhs.quality_loss) < eps;
}

/// @brief Inequality operator.
inline bool operator!=(const resources_t &lhs, const resources_t &rhs) noexcept
{
    return !(lhs == rhs);
}

/// @brief Less-than operator.
inline bool operator<(const resources_t &lhs, const resources_t &rhs) noexcept
{
    // Compare by total cost first, then by time
    double lhs_total = lhs.total_cost();
    double rhs_total = rhs.total_cost();
    if (std::abs(lhs_total - rhs_total) > 1e-9) {
        return lhs_total < rhs_total;
    }
    return lhs.time < rhs.time;
}

/// @brief Stream output operator.
inline std::ostream &operator<<(std::ostream &os, const resources_t &resources)
{
    return os << resources.to_string();
}

} // namespace manufacturing

namespace json
{

/// @brief JSON serialization for resources_t.
inline json::jnode_t &operator<<(json::jnode_t &lhs, const manufacturing::resources_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["time"] << rhs.time;
    lhs["energy"] << rhs.energy;
    lhs["energy_cost"] << rhs.energy_cost;
    lhs["material_cost"] << rhs.material_cost;
    lhs["labor_cost"] << rhs.labor_cost;
    lhs["equipment_wear"] << rhs.equipment_wear;
    lhs["quality_loss"] << rhs.quality_loss;
    return lhs;
}

/// @brief JSON deserialization for resources_t.
inline const json::jnode_t &operator>>(const json::jnode_t &lhs, manufacturing::resources_t &rhs)
{
    lhs["time"] >> rhs.time;
    lhs["energy"] >> rhs.energy;
    lhs["energy_cost"] >> rhs.energy_cost;
    lhs["material_cost"] >> rhs.material_cost;
    lhs["labor_cost"] >> rhs.labor_cost;
    lhs["equipment_wear"] >> rhs.equipment_wear;
    lhs["quality_loss"] >> rhs.quality_loss;
    return lhs;
}

} // namespace json
