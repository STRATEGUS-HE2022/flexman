/// @file resources.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Defines resource usage metrics for tapping operations.
///
/// @details
/// This file defines the `resources_t` structure, which tracks the energy and
/// time consumed during tapping operations. It provides:
/// - Operators for comparison (`==`, `!=`, `<`, `<=`) to facilitate resource evaluation.
/// - Stream output operator for formatted printing.
/// - JSON serialization and deserialization functions for easy data exchange.
///
/// These utilities support performance analysis and optimization in the
/// tapping system.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#include <fsmlib/feq.hpp>
#include <json/json.hpp>

#include <iomanip>
#include <iostream>

namespace tapping
{

/// @brief Tapping resources.
struct resources_t {
    double energy; ///< Energy spent tapping.
    double time;   ///< Time spent tapping.
};

inline bool operator==(const resources_t &lhs, const resources_t &rhs) noexcept
{
    return fsmlib::feq::approximately_equal(lhs.energy, rhs.energy) &&
           fsmlib::feq::approximately_equal(lhs.time, rhs.time);
}

inline bool operator!=(const resources_t &lhs, const resources_t &rhs) noexcept
{
    return !fsmlib::feq::approximately_equal(lhs.energy, rhs.energy);
}

inline bool operator<=(const resources_t &lhs, const resources_t &rhs) noexcept
{
    return fsmlib::feq::approximately_lesser_than_equal(lhs.energy, rhs.energy) &&
           fsmlib::feq::approximately_lesser_than_equal(lhs.time, rhs.time);
}

inline bool operator<(const resources_t &lhs, const resources_t &rhs) noexcept
{
    if (!fsmlib::feq::approximately_equal(lhs.energy, rhs.energy))
        return lhs.energy < rhs.energy;
    return lhs.time < rhs.time;
}

inline std::ostream &operator<<(std::ostream &lhs, const resources_t &rhs)
{
    lhs << std::fixed;
    lhs << "(";
    lhs << std::setprecision(3) << std::setw(6) << std::right << rhs.time << ",";
    lhs << std::setprecision(3) << std::setw(8) << std::right << rhs.energy;
    lhs << ")";
    return lhs;
}

} // namespace tapping

namespace json
{

inline json::jnode_t &operator<<(json::jnode_t &lhs, const tapping::resources_t &rhs)
{
    lhs.set_type(json::JTYPE_OBJECT);
    lhs["energy"] << rhs.energy;
    lhs["time"] << rhs.time;
    return lhs;
}

inline const json::jnode_t &operator>>(const json::jnode_t &lhs, tapping::resources_t &rhs)
{
    lhs["energy"] >> rhs.energy;
    lhs["time"] >> rhs.time;
    return lhs;
}

} // namespace json
