/// @file plotting.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Provides plotting utilities for visualizing simulation results and Pareto fronts.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#include <array>
#include <vector>
#include <limits> // Required for std::numeric_limits
#include <iomanip> // Required for std::setprecision, std::fixed, std::setw, std::right
#include <sstream> // Required for std::stringstream

#include <gpcpp/gnuplot.hpp>

#include "defines.hpp"

namespace heating
{

/// @brief Builds the plot name string for a Pareto front.
/// @param pareto The Pareto front object containing metadata such as the step length.
/// @return std::string A formatted string representing the name of the Pareto front.
inline std::string build_plot_name(const heating::pareto_front_t &pareto)
{
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed << "Pareto [" << std::right << std::setw(6) << pareto.step_length << "]";
    return ss.str();
}

/// @brief Extracts time and energy data from a list of solutions.
/// @param solutions A vector of solutions, each containing time and energy
/// resources.
/// @return A pair of vectors: the first contains time data, and the second
/// contains energy data.
inline std::pair<std::vector<double>, std::vector<double>> extract_time_energy(const std::vector<solution_t> &solutions)
{
    std::vector<double> time, energy;
    for (const auto &solution : solutions) {
        time.push_back(solution.resources.time);
        energy.push_back(solution.resources.energy);
    }
    return {time, energy};
}

/// @brief Extracts time and state data from a list of solutions.
/// @param solutions A vector of solutions, each containing time and state
/// (temperature) data.
/// @return A pair of vectors: the first contains time data, and the second
/// contains temperature data.
inline std::pair<std::vector<double>, std::vector<double>> extract_time_state(const std::vector<solution_t> &solutions)
{
    std::vector<double> time, temperature;
    for (const auto &solution : solutions) {
        time.push_back(solution.resources.time);
        temperature.push_back(solution.state[0]); // Assuming state[0] is temperature
    }
    return {time, temperature};
}

/// @brief Computes global axis limits for a dataset containing multiple Pareto fronts.
/// @param results The result set containing multiple Pareto fronts.
/// @param margin_fraction The fraction of the range to use as margin.
/// @return A pair of pairs: ((x_min, x_max), (y_min, y_max)) for x and y limits.
std::pair<std::array<double, 2>, std::array<double, 2>>
compute_global_limits(const heating::result_t &results, double margin_fraction = 0.1)
{
    // Initialize min and max values with extreme values
    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::lowest();
    double y_min = std::numeric_limits<double>::max();
    double y_max = std::numeric_limits<double>::lowest();

    // Aggregate all time and energy data
    for (const auto &pareto : results.pareto_fronts) {
        for (const auto &solution : pareto.solutions) {
            double time   = solution.resources.time;
            double energy = solution.resources.energy;

            x_min = std::min(x_min, time);
            x_max = std::max(x_max, time);
            y_min = std::min(y_min, energy);
            y_max = std::max(y_max, energy);
        }
    }

    // Handle edge case where all data points are the same
    if (fsmlib::feq::approximately_equal(x_min, x_max)) {
        double margin = (!fsmlib::feq::approximately_equal_to_zero(x_min)) ? std::abs(x_min) * margin_fraction : 1.0;
        x_min -= margin;
        x_max += margin;
    }
    if (fsmlib::feq::approximately_equal(y_min, y_max)) {
        double margin = (!fsmlib::feq::approximately_equal_to_zero(y_min)) ? std::abs(y_min) * margin_fraction : 1.0;
        y_min -= margin;
        y_max += margin;
    }

    // Add margin to the range
    double x_range = x_max - x_min;
    double y_range = y_max - y_min;

    double x_margin = x_range * margin_fraction;
    double y_margin = y_range * margin_fraction;

    return {{x_min - x_margin, x_max + x_margin}, {y_min - y_margin, y_max + y_margin}};
}

/// @brief Plots the Pareto fronts from the given results.
///
/// @details This function generates a plot of Pareto fronts extracted from the
/// results, where each Pareto front represents a trade-off curve between time
/// and energy. It assigns a unique color to each Pareto front and displays a
/// legend for clarity.
/// @param results The result set containing multiple Pareto fronts to be
/// plotted. Each Pareto front includes solutions with associated time and
/// energy data.
inline void plot_pareto_front(const heating::result_t &results)
{
    gpcpp::Gnuplot gp;

    // Set terminal and output format
    gp.set_terminal(gpcpp::terminal_type_t::wxt);

    // Enable minor tics.
    gp.set_xtics_minor(2);
    gp.set_ytics_minor(2);

    // Configure major grid.
    gp.set_grid_line_type(gpcpp::grid_type_t::major, gpcpp::line_type_t::solid, gpcpp::Color("black"), 0.5);
    // Configure minor grid.
    gp.set_grid_line_type(gpcpp::grid_type_t::minor, gpcpp::line_type_t::dashed, gpcpp::Color("black"), 0.25);
    // Apply grid configuration.
    gp.apply_grid("xtics ytics mxtics mytics", "back");

    // Compute global axis limits for all Pareto fronts.
    auto [x_limits, y_limits] = compute_global_limits(results);

    // Set limits for the x and y axes.
    gp.set_xrange(x_limits[0], x_limits[1]);
    gp.set_yrange(y_limits[0], y_limits[1]);

    // Label the axes.
    gp.set_xlabel("Time (s)");
    gp.set_ylabel("Energy (W)");

    // Enable the key (legend) and set its properties.
    gp.set_legend("top right", "", "Pareto Fronts", true, 1.0, 2.0);

    // Iterate over each Pareto front and add its data to the plot.
    for (const auto &pareto : results.pareto_fronts) {
        // Extract the time and energy data from the current Pareto front.
        auto [time, energy] = extract_time_energy(pareto.solutions);

        // Create a step plot (stairs) for the current Pareto front.
        gp.plot_xy(time, energy, build_plot_name(pareto)).set_plot_type(gpcpp::plot_type_t::steps).set_line_width(2.0);
    }

    gp.show();
}

inline void plot_simulations(const std::vector<heating::simulation_t> &simulations)
{
    gpcpp::Gnuplot gp;

    // Set up the Gnuplot terminal and output format
    gp.set_terminal(gpcpp::terminal_type_t::wxt);

    // Enable minor ticks for better visualization
    gp.set_xtics_minor(2);
    gp.set_ytics_minor(2);

    // Configure grid lines
    gp.set_grid_line_type(gpcpp::grid_type_t::major, gpcpp::line_type_t::solid, gpcpp::Color("black"), 0.5);
    gp.set_grid_line_type(gpcpp::grid_type_t::minor, gpcpp::line_type_t::dashed, gpcpp::Color("black"), 0.25);
    gp.apply_grid("xtics ytics mxtics mytics", "back");

    // Define axis labels
    gp.set_xlabel("Time (s)");
    gp.set_ylabel("Temperature (C)"); // Changed from Depth to Temperature

    // Enable the legend
    gp.set_legend("top right", "", "Simulations", true, 1.0, 2.0);

    // Plot each simulation's Pareto front
    for (const auto &simulation : simulations) {
        // Extract the time and temperature data from the current simulation
        auto [time, temperature] = extract_time_state(simulation.data.evolution); // Changed from extract_time_depth

        // Plot the Pareto front
        gp.plot_xy(time, temperature, simulation.name)      // Plot data
            .set_plot_type(gpcpp::plot_type_t::lines) // Changed from steps to lines, as temperature is continuous
            .set_line_width(2.0);                     // Adjust line width
    }

    gp.show();
}

} // namespace heating