/// @file plotting.hpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Plotting functions for manufacturing system results.
///
/// @details
/// This file provides plotting functionality for visualizing manufacturing
/// system search results and simulations using Gnuplot.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#pragma once

#include "defines.hpp"

#include <gpcpp/gnuplot.hpp>
#include <sstream>
#include <vector>

namespace manufacturing
{

/// @brief Plot Pareto front for manufacturing optimization results.
inline void plot_pareto_front(const result_t &results)
{
    if (results.pareto_fronts.empty()) {
        std::cerr << "No Pareto fronts to plot.\n";
        return;
    }

    try {
        gpcpp::Gnuplot gp;

        // Configure plot
        gp << "set title 'Manufacturing System - Pareto Front: Time vs Total Cost'";
        gp << "set xlabel 'Time (s)'";
        gp << "set ylabel 'Total Cost (units)'";
        gp << "set grid";
        gp << "set key top right";

        std::vector<std::string> colors = {"red", "blue", "green", "orange", "purple", "brown"};

        for (std::size_t front_idx = 0; front_idx < results.pareto_fronts.size() && front_idx < colors.size(); ++front_idx) {
            const auto &front = results.pareto_fronts[front_idx];

            if (front.solutions.empty()) {
                continue;
            }

            // Extract time and total cost data
            std::vector<double> times, costs;
            for (const auto &solution : front.solutions) {
                times.push_back(solution.resources.time);
                costs.push_back(solution.resources.total_cost());
            }

            std::ostringstream title;
            title << "Pareto Front " << (front_idx + 1)
                  << " (step: " << front.step_length << "s)";

            gp.plot_xy(times, costs, title.str())
                .set_line_type(gpcpp::line_type_t::solid)
                .set_line_color(colors[front_idx])
                .set_point_type(gpcpp::point_type_t::filled_circle)
                .set_point_size(1.5);
        }

        gp.show();

    } catch (const std::exception &e) {
        std::cerr << "Error plotting Pareto front: " << e.what() << "\n";
    }
}

/// @brief Plot detailed cost breakdown for a Pareto front.
inline void plot_cost_breakdown(const result_t &results)
{
    if (results.pareto_fronts.empty() || results.pareto_fronts[0].solutions.empty()) {
        std::cerr << "No solutions to plot cost breakdown.\n";
        return;
    }

    try {
        gpcpp::Gnuplot gp;

        const auto &solutions = results.pareto_fronts[0].solutions;

        // Extract cost components
        std::vector<double> indices, energy_costs, material_costs, labor_costs, wear_costs, quality_costs;

        for (std::size_t i = 0; i < solutions.size(); ++i) {
            indices.push_back(static_cast<double>(i + 1));
            energy_costs.push_back(solutions[i].resources.energy_cost);
            material_costs.push_back(solutions[i].resources.material_cost);
            labor_costs.push_back(solutions[i].resources.labor_cost);
            wear_costs.push_back(solutions[i].resources.equipment_wear);
            quality_costs.push_back(solutions[i].resources.quality_loss);
        }

        gp << "set title 'Manufacturing Cost Breakdown by Solution'";
        gp << "set xlabel 'Solution Index'";
        gp << "set ylabel 'Cost (units)'";
        gp << "set grid";
        gp << "set key top left";
        gp << "set style fill solid 0.7";

        // Stacked bar chart would be ideal, but for simplicity, use separate lines
        gp.plot_xy(indices, energy_costs, "Energy Cost")
            .set_line_type(gpcpp::line_type_t::solid)
            .set_line_color("red")
            .set_point_type(gpcpp::point_type_t::filled_circle)
            .set_point_size(5);

        gp.plot_xy(indices, material_costs, "Material Cost")
            .set_line_type(gpcpp::line_type_t::solid)
            .set_line_color("blue")
            .set_point_type(gpcpp::point_type_t::filled_circle)
            .set_point_size(6);

        gp.plot_xy(indices, labor_costs, "Labor Cost")
            .set_line_type(gpcpp::line_type_t::solid)
            .set_line_color("green")
            .set_point_type(gpcpp::point_type_t::filled_circle)
            .set_point_size(7);

        gp.plot_xy(indices, wear_costs, "Equipment Wear")
            .set_line_type(gpcpp::line_type_t::solid)
            .set_line_color("orange")
            .set_point_type(gpcpp::point_type_t::filled_circle)
            .set_point_size(8);

        gp.plot_xy(indices, quality_costs, "Quality Loss")
            .set_line_type(gpcpp::line_type_t::solid)
            .set_line_color("purple")
            .set_point_type(gpcpp::point_type_t::filled_circle)
            .set_point_size(9);

        gp.show();

    } catch (const std::exception &e) {
        std::cerr << "Error plotting cost breakdown: " << e.what() << "\n";
    }
}

/// @brief Plot simulation results for manufacturing processes.
inline void plot_simulations(const std::vector<simulation_t> &simulations)
{
    if (simulations.empty()) {
        std::cerr << "No simulations to plot.\n";
        return;
    }

    try {
        // Plot state evolution over time
        gpcpp::Gnuplot gp_states;

        gp_states << "set title 'Manufacturing Process - State Evolution'";
        gp_states << "set xlabel 'Time (s)'";
        gp_states << "set ylabel 'State Values'";
        gp_states << "set grid";
        gp_states << "set key top right";

        std::vector<std::string> state_names = {
            "Surface Roughness (μm)",
            "Hardness (HRC)",
            "Temperature (°C)",
            "Dimensional Accuracy (μm)",
            "Coating Thickness (μm)",
            "Stress Level (MPa)"};

        std::vector<std::string> colors = {"red", "blue", "green", "orange", "purple", "brown"};

        for (std::size_t sim_idx = 0; sim_idx < std::min(simulations.size(), size_t(3)); ++sim_idx) {
            const auto &sim = simulations[sim_idx];

            if (sim.data.evolution.empty()) {
                continue;
            }

            // For each state variable, plot its evolution
            for (std::size_t state_idx = 0; state_idx < n_states; ++state_idx) {
                std::vector<double> times, values;

                for (std::size_t step = 0; step < sim.data.evolution.size(); ++step) {
                    times.push_back(sim.data.evolution[step].resources.time);
                    values.push_back(sim.data.evolution[step].state[state_idx]);
                }

                std::ostringstream title;
                title << sim.name << " - " << state_names[state_idx];

                gp_states.plot_xy(times, values, title.str())
                    .set_line_type(gpcpp::line_type_t::solid)
                    .set_line_color(colors[state_idx % colors.size()]);
            }
        }

        gp_states.show();

        // Plot cost accumulation
        gpcpp::Gnuplot gp_costs;

        gp_costs << "set title 'Manufacturing Process - Cost Accumulation'";
        gp_costs << "set xlabel 'Time (s)'";
        gp_costs << "set ylabel 'Cumulative Cost (units)'";
        gp_costs << "set grid";
        gp_costs << "set key top left";

        for (std::size_t sim_idx = 0; sim_idx < std::min(simulations.size(), size_t(5)); ++sim_idx) {
            const auto &sim = simulations[sim_idx];

            if (sim.data.evolution.empty()) {
                continue;
            }

            std::vector<double> times, total_costs;

            for (const auto &step : sim.data.evolution) {
                times.push_back(step.resources.time);
                total_costs.push_back(step.resources.total_cost());
            }

            gp_costs.plot_xy(times, total_costs, sim.name)
                .set_line_type(gpcpp::line_type_t::solid)
                .set_line_color(colors[sim_idx % colors.size()]);
        }

        gp_costs.show();

    } catch (const std::exception &e) {
        std::cerr << "Error plotting simulations: " << e.what() << "\n";
    }
}

/// @brief Plot machine utilization analysis.
inline void plot_machine_utilization(const result_t &results)
{
    if (results.pareto_fronts.empty() || results.pareto_fronts[0].solutions.empty()) {
        std::cerr << "No solutions to analyze machine utilization.\n";
        return;
    }

    try {
        gpcpp::Gnuplot gp;

        // Count machine usage across all solutions
        std::map<flexman::core::ModeId, std::size_t> machine_usage;

        for (const auto &front : results.pareto_fronts) {
            for (const auto &solution : front.solutions) {
                for (const auto &execution : solution.sequence) {
                    machine_usage[execution.mode] += execution.times;
                }
            }
        }

        std::vector<double> machine_ids, usage_counts;
        for (const auto &usage : machine_usage) {
            machine_ids.push_back(static_cast<double>(usage.first));
            usage_counts.push_back(static_cast<double>(usage.second));
        }

        gp << "set title 'Machine Utilization Across All Solutions'";
        gp << "set xlabel 'Machine ID'";
        gp << "set ylabel 'Usage Count'";
        gp << "set grid";
        gp << "set style fill solid 0.7";
        gp << "set boxwidth 0.8";

        gp.plot_xy(machine_ids, usage_counts, "Machine Usage")
            .set_line_type(gpcpp::line_type_t::solid)
            .set_line_color("blue");

        gp.show();

    } catch (const std::exception &e) {
        std::cerr << "Error plotting machine utilization: " << e.what() << "\n";
    }
}

} // namespace manufacturing
