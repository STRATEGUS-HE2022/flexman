/// @file plotting.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Plotting functions.

#include <flexman/simulation/simulate.hpp>
#include <flexman/serialization.hpp>
#include <flexman/logging.hpp>
#include <gpcpp/gnuplot.hpp>
#include <vector>
#include <array>

#include "defines.hpp"

namespace tapping
{

/// @brief Converts a color vector to an RGB array.
///
/// @param color The vector containing the RGB color values. Must have exactly 3 elements.
///
/// @return std::array<float, 3> An array containing the color values as floats.
///
/// @throws std::runtime_error If the input vector does not have exactly 3 elements.
inline std::array<float, 3> color_to_rgb(const std::vector<double> &color)
{
    if (color.size() != 3) {
        throw std::runtime_error("Color not RGB.");
    }
    return std::array<float, 3>{ static_cast<float>(color[0]), static_cast<float>(color[1]), static_cast<float>(color[2]) };
}

/// @brief Builds the plot name string for a Pareto front.
///
/// @param pareto The Pareto front object containing metadata such as the step length.
///
/// @return std::string A formatted string representing the name of the Pareto front.
inline std::string build_plot_name(const tapping::pareto_front_t &pareto)
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
    return { time, energy };
}

/// @brief Extracts time and depth data from a list of solutions.
/// @param solutions A vector of solutions, each containing time and depth
/// resources.
/// @return A pair of vectors: the first contains time data, and the second
/// contains depth data.
inline std::pair<std::vector<double>, std::vector<double>> extract_time_depth(const std::vector<solution_t> &solutions)
{
    std::vector<double> time, depth;
    for (const auto &solution : solutions) {
        time.push_back(solution.resources.time);
        depth.push_back(solution.state[2]);
    }
    return { time, depth };
}

/// @brief Rounds a number to the nearest multiple of a given value.
/// @param value The number to round.
/// @param multiple The multiple to round to.
/// @return The rounded number.
double round_to_multiple(double value, double multiple)
{
    if (multiple == 0.0) {
        return value; // No rounding needed if multiple is 0
    }
    return std::round(value / multiple) * multiple;
}

/// @brief Computes global axis limits for a dataset containing multiple Pareto fronts.
///
/// @param results The result set containing multiple Pareto fronts.
/// @param margin_fraction The fraction of the range to use as margin.
///
/// @return A pair of pairs: ((x_min, x_max), (y_min, y_max)) for x and y limits.
std::pair<std::array<double, 2>, std::array<double, 2>> compute_global_limits(const tapping::result_t &results, double margin_fraction = 0.1)
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
    if (x_min == x_max) {
        double margin = (x_min != 0.0) ? std::abs(x_min) * margin_fraction : 1.0;
        x_min -= margin;
        x_max += margin;
    }
    if (y_min == y_max) {
        double margin = (y_min != 0.0) ? std::abs(y_min) * margin_fraction : 1.0;
        y_min -= margin;
        y_max += margin;
    }

    // Add margin to the range
    double x_range = x_max - x_min;
    double y_range = y_max - y_min;

    double x_margin = x_range * margin_fraction;
    double y_margin = y_range * margin_fraction;

    return { { x_min - x_margin, x_max + x_margin }, { y_min - y_margin, y_max + y_margin } };
}

/// @brief Plots the Pareto fronts from the given results.
///
/// @details This function generates a plot of Pareto fronts extracted from the
/// results, where each Pareto front represents a trade-off curve between time
/// and energy. It assigns a unique color to each Pareto front and displays a
/// legend for clarity.
///
/// @param results The result set containing multiple Pareto fronts to be
/// plotted. Each Pareto front includes solutions with associated time and
/// energy data.
inline void plot_pareto_front(const tapping::result_t &results)
{
    gpcpp::Gnuplot gp;

    // Set terminal and output format
    gp.set_terminal(gpcpp::terminal_type_t::wxt);

    // Enable minor tics.
    gp.set_xtics_minor(2);
    gp.set_ytics_minor(2);

    // Configure major grid.
    gp.set_grid_line_style(gpcpp::grid_type_t::major, gpcpp::line_style_t::solid, gpcpp::Color("black"), 0.5);
    // Configure minor grid.
    gp.set_grid_line_style(gpcpp::grid_type_t::minor, gpcpp::line_style_t::dashed, gpcpp::Color("black"), 0.25);
    // Apply grid configuration.
    gp.apply_grid("xtics ytics mxtics mytics", -1, "back");

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
        gp.plot_xy(time, energy, build_plot_name(pareto))
            .set_plot_style(gpcpp::plot_style_t::steps)
            .set_line_width(2.0);
    }

    gp.show();
}

inline void plot_simulations(const std::vector<tapping::simulation_t> &simulations)
{
    gpcpp::Gnuplot gp;

    // Set up the Gnuplot terminal and output format
    gp.set_terminal(gpcpp::terminal_type_t::wxt);

    // Enable minor ticks for better visualization
    gp.set_xtics_minor(2);
    gp.set_ytics_minor(2);

    // Configure grid lines
    gp.set_grid_line_style(gpcpp::grid_type_t::major, gpcpp::line_style_t::solid, gpcpp::Color("black"), 0.5);
    gp.set_grid_line_style(gpcpp::grid_type_t::minor, gpcpp::line_style_t::dashed, gpcpp::Color("black"), 0.25);
    gp.apply_grid("xtics ytics mxtics mytics", -1, "back");

    // Define axis labels
    gp.set_xlabel("Time (s)");
    gp.set_ylabel("Depth (mm)");

    // Enable the legend
    gp.set_legend("top right", "", "Simulations", true, 1.0, 2.0);

    // Plot each simulation's Pareto front
    for (const auto &simulation : simulations) {
        // Extract the time and depth data from the current simulation
        auto [time, depth] = extract_time_depth(simulation.data.evolution);

        // Plot the Pareto front
        gp.plot_xy(time, depth, simulation.name)        // Plot data
            .set_plot_style(gpcpp::plot_style_t::steps) // Use step plot
            .set_line_width(2.0);                       // Adjust line width
    }

    // Display the completed plot
    gp.show();
}

} // namespace tapping
