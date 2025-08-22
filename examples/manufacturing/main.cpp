/// @file main.cpp
/// @author Enrico Fraccaroli (enrico.fraccaroli@univr.it)
///
/// @brief Entry point for the multi-machine manufacturing system search and simulation.
///
/// @details
/// This program demonstrates a multi-machine manufacturing system where each mode
/// represents a different machine operating on different aspects of a workpiece.
/// The system includes:
/// - Milling Machine: Affects surface roughness and dimensional accuracy
/// - Heat Treatment: Affects hardness and stress relief
/// - Coating Station: Applies protective coatings
/// - Polishing Machine: Improves surface finish
/// - Quality Inspection: Measurement and verification
/// - Cooling Station: Temperature control
///
/// Each machine has different operation modes (Light, Standard, Intensive) with
/// varying cost-performance trade-offs. The scheduler synthesis finds optimal
/// sequences of machine operations to achieve target workpiece properties while
/// minimizing time and cost.
///
/// @copyright Copyright (c) 2024-2025 Enrico Fraccaroli, University of Verona,
/// University of North Carolina at Chapel Hill. Distributed under the BSD
/// 3-Clause License. See LICENSE.md for details.
///

#include <cmath>
#include <cmdlp/parser.hpp>
#include <flexman/serialization.hpp>

#include "builder.hpp"
#include "defines.hpp"
#include "plotting.hpp"
#include "search.hpp"

namespace manufacturing
{

enum run_option : unsigned char {
    run_search,
    run_simulation,
};

enum mode_option : unsigned char {
    mode_discrete,
    mode_continuous,
};

enum algorithm_type : unsigned char {
    algorithm_heuristic,
    algorithm_exhaustive,
    algorithm_single_machine,
};

/// @brief Compares two solutions for ascending order based on time and total cost.
inline auto compare_ascending(const solution_t &lhs, const solution_t &rhs) -> bool
{
    // Primary sort: by time (ascending).
    if (!fsmlib::feq::approximately_equal(lhs.resources.time, rhs.resources.time)) {
        return lhs.resources.time < rhs.resources.time;
    }
    // Secondary sort: by total cost (ascending).
    return lhs.resources.total_cost() < rhs.resources.total_cost();
}

/// @brief Logs the details of each Pareto front and its solutions.
inline void log_results(quire::log_level log_level, const result_t &results)
{
    qlog(flexman::logging::app, log_level, "============================================================\n");
    for (const auto &pareto : results.pareto_fronts) {
        qlog(
            flexman::logging::app, log_level,
            "Pareto front (step: %8.3f s, runtime: %8.3f s):\n",
            pareto.step_length, pareto.runtime);

        for (const auto &solution : pareto.solutions) {
            qlog(flexman::logging::app, log_level, "\t%s\n", solution.to_string().c_str());

            // Log the machine sequence
            std::ostringstream seq_stream;
            seq_stream << "\t  Sequence: ";
            for (const auto &execution : solution.sequence) {
                seq_stream << "M" << execution.mode << "*" << execution.times << " ";
            }
            qlog(flexman::logging::app, log_level, "%s\n", seq_stream.str().c_str());

            // Log final state values
            qlog(flexman::logging::app, log_level, "\t  Final State: SR=%.2f, H=%.1f, T=%.1f°C, DA=%.2f, CT=%.1f, SL=%.1f\n", solution.state[0], solution.state[1], solution.state[2], solution.state[3], solution.state[4], solution.state[5]);
        }
    }
    qlog(flexman::logging::app, log_level, "============================================================\n");
}

/// @brief Compare results between different algorithm runs.
template <typename State, typename Resources>
void compare_results(
    const flexman::core::Result<State, Resources> &result1,
    const flexman::core::Result<State, Resources> &result2)
{
    if (result1.pareto_fronts.size() != result2.pareto_fronts.size()) {
        qwarning(
            flexman::logging::app, "Results differ in the number of Pareto fronts (%u vs %u)\n.",
            result1.pareto_fronts.size(), result2.pareto_fronts.size());
        return;
    }

    for (size_t i = 0; i < result1.pareto_fronts.size(); ++i) {
        const auto &front1 = result1.pareto_fronts[i];
        const auto &front2 = result2.pareto_fronts[i];

        if (front1.solutions.size() != front2.solutions.size()) {
            qwarning(
                flexman::logging::app, "Pareto front %u differ in the number of solutions (%u vs %u)\n.",
                i + 1, front1.solutions.size(), front2.solutions.size());
            continue;
        }

        for (size_t j = 0; j < front1.solutions.size(); ++j) {
            const auto &sol1 = front1.solutions[j];
            const auto &sol2 = front2.solutions[j];

            qinfo(
                flexman::logging::app,
                "Solution %2u in Front %2u: Time %.3f->%.3f, Cost %.3f->%.3f\n",
                j + 1, i + 1, sol1.resources.time, sol2.resources.time,
                sol1.resources.total_cost(), sol2.resources.total_cost());
        }
    }
}

/// @brief Save results to JSON file.
template <typename SearchManager, typename Parameter, typename Mode>
inline void save_results(
    const SearchManager &manager,
    const result_t &results,
    const std::vector<Parameter> &parameters,
    const std::vector<Mode> &modes,
    const std::string &filename)
{
    json::jnode_t root;
    root.set_type(json::JTYPE_OBJECT);
    root["manager"] << manager;
    root["results"] << results;
    root["modes"].clear();
    root["modes"].set_type(json::JTYPE_ARRAY);
    root["modes"].resize(parameters.size());
    for (std::size_t i = 0; i < parameters.size(); ++i) {
        root["modes"][i].set_type(json::JTYPE_OBJECT);
        root["modes"][i]["parameters"] << parameters[i];
        root["modes"][i]["mode"] << modes[i];
    }
    if (!json::parser::write_file(filename, root, true, 4U)) {
        std::cerr << "Failed to save to `" << filename << "`.\n";
    }
}

/// @brief Setup command line option parser.
void setup_option_parser(cmdlp::Parser &parser)
{
    parser.addToggle("-h", "--help", "Show this help.", false);

    parser.addMultiOption(
        "-r", "--run", "Run (0) search, (1) simulation.",
        {std::to_string(run_search), std::to_string(run_simulation)},
        std::to_string(run_search));

    parser.addMultiOption(
        "-m", "--mode", "Run (0) discrete, (1) continuous.",
        {std::to_string(mode_discrete), std::to_string(mode_continuous)},
        std::to_string(mode_discrete));

    parser.addMultiOption(
        "-a", "--algorithm", "Run (0) heuristic, (1) exhaustive, (2) single machine.",
        {std::to_string(algorithm_heuristic), std::to_string(algorithm_exhaustive), std::to_string(algorithm_single_machine)},
        std::to_string(algorithm_heuristic));

    // Post-search optimization
    parser.addToggle("-p", "--pso", "Enable post-search optimization using PSO", false);
    parser.addOption("-pn", "--pso_num_particles", "Number of particles in the PSO swarm", false, 100);
    parser.addOption("-pm", "--pso_max_iterations", "Maximum number of iterations for PSO", false, 50);
    parser.addOption("-pi", "--pso_inertia", "Inertia weight for PSO", false, 0.2);
    parser.addOption("-pc", "--pso_cognitive", "Cognitive weight for PSO", false, 0.4);
    parser.addOption("-ps", "--pso_social", "Social weight for PSO", false, 0.4);

    // Output file
    parser.addOption("-o", "--output", "The file where the execution results are saved", false, "manufacturing_output.json");

    // Initial workpiece state
    parser.addOption("-isr", "--initial_surface_roughness", "Initial surface roughness (μm)", false, 10.0);
    parser.addOption("-inh", "--initial_hardness", "Initial hardness (HRC)", false, 20.0);
    parser.addOption("-int", "--initial_temperature", "Initial temperature (°C)", false, 25.0);
    parser.addOption("-ida", "--initial_dimensional_accuracy", "Initial dimensional accuracy (μm)", false, 5.0);
    parser.addOption("-ict", "--initial_coating_thickness", "Initial coating thickness (μm)", false, 0.0);
    parser.addOption("-isl", "--initial_stress_level", "Initial stress level (MPa)", false, 50.0);

    // Target workpiece state
    parser.addOption("-tsr", "--target_surface_roughness", "Target surface roughness (μm)", false, 2.0);
    parser.addOption("-trh", "--target_hardness", "Target hardness (HRC)", false, 45.0);
    parser.addOption("-trt", "--target_temperature", "Target temperature (°C)", false, 25.0);
    parser.addOption("-tda", "--target_dimensional_accuracy", "Target dimensional accuracy (μm)", false, 0.5);
    parser.addOption("-tct", "--target_coating_thickness", "Target coating thickness (μm)", false, 20.0);
    parser.addOption("-tsl", "--target_stress_level", "Target stress level (MPa)", false, 10.0);

    // Search parameters
    parser.addOption("-tm", "--time_max", "The maximum simulated time", false, 300.0);
    parser.addOption("-td", "--time_delta", "The time delta", false, 1.0);
    parser.addOption("-th", "--threshold", "Completion threshold", false, 0.1);
    parser.addOption("-dl", "--timeout", "Algorithm timeout (seconds)", false, 120.0);
    parser.addToggle("-in", "--interactive", "Enable the interactive mode", false);

    // Search manager parameters
    parser.addOption("-it", "--iterations", "The number of iterations for the search", false, 8U);
    parser.addOption("-cf", "--coarsening_factor", "The factor by which the step length is coarsened", false, 2U);

    // Logging and plotting
    parser.addMultiOption(
        "-lg", "--log_level", "The log level",
        {
            std::to_string(quire::log_level::debug),
            std::to_string(quire::log_level::info),
            std::to_string(quire::log_level::warning),
            std::to_string(quire::log_level::error),
            std::to_string(quire::log_level::critical),
        },
        std::to_string(quire::log_level::info));
    parser.addToggle("-pl", "--plot", "Plot the results", false);
}

/// @brief Create all machine modes for the manufacturing system.
template <typename ModeType>
std::vector<ModeType> create_machine_modes(double time_delta = 1.0)
{
    std::vector<ModeType> modes;
    std::vector<parameters_t> parameters;

    flexman::core::ModeId mode_id = 0;

    // Create modes for each machine type and operation mode combination
    for (int machine = 0; machine < 6; ++machine) {
        for (int operation = 0; operation < 3; ++operation) {
            auto machine_type   = static_cast<MachineType>(machine);
            auto operation_mode = static_cast<OperationMode>(operation);

            parameters_t params(machine_type, operation_mode);
            builder_t builder(params);

            ModeType mode;
            if constexpr (std::is_same_v<ModeType, discrete_mode_t>) {
                mode = builder.make_discrete_mode(mode_id, time_delta);
            } else {
                mode = builder.make_continuous_mode(mode_id);
            }

            modes.emplace_back(mode);
            parameters.emplace_back(params);
            ++mode_id;

            qinfo(flexman::logging::app, "Created Mode %u: %s\n", mode_id - 1, mode.machine_name.c_str());
        }
    }

    return modes;
}

/// @brief Execute discrete mode search and simulation.
auto execute_in_discrete_mode(cmdlp::Parser &parser) -> int
{
    // Setup search parameters
    discrete_search_t search;
    search.initial_state = {
        parser.getOption<double>("--initial_surface_roughness"),
        parser.getOption<double>("--initial_hardness"),
        parser.getOption<double>("--initial_temperature"),
        parser.getOption<double>("--initial_dimensional_accuracy"),
        parser.getOption<double>("--initial_coating_thickness"),
        parser.getOption<double>("--initial_stress_level")};
    search.target_state = {
        parser.getOption<double>("--target_surface_roughness"),
        parser.getOption<double>("--target_hardness"),
        parser.getOption<double>("--target_temperature"),
        parser.getOption<double>("--target_dimensional_accuracy"),
        parser.getOption<double>("--target_coating_thickness"),
        parser.getOption<double>("--target_stress_level")};
    search.time_max          = parser.getOption<double>("--time_max");
    search.time_delta        = parser.getOption<double>("--time_delta");
    search.threshold         = parser.getOption<double>("--threshold");
    search.timeout           = parser.getOption<double>("--timeout");
    search.interactive       = parser.getOption<bool>("--interactive");
    search.coarsening_factor = parser.getOption<unsigned>("--coarsening_factor");

    auto algorithm  = parser.getOption<unsigned>("-a");
    auto iterations = parser.getOption<unsigned>("--iterations");

    // Create all machine modes
    auto modes = create_machine_modes<discrete_mode_t>(search.time_delta);
    std::vector<parameters_t> parameters; // For saving results

    qinfo(flexman::logging::app, "Created %u discrete machine modes.\n", modes.size());

    if (parser.getOption<unsigned>("--run") == run_search) {
        result_t results;

        qinfo(flexman::logging::app, "Searching for optimal manufacturing sequences...\n");
        if (algorithm == algorithm_heuristic) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Heuristic>(
                &search, modes, iterations);
        } else if (algorithm == algorithm_exhaustive) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Exhaustive>(
                &search, modes, iterations);
        } else if (algorithm == algorithm_single_machine) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::SingleMachine>(
                &search, modes, iterations);
        }

        // Sort results
        qinfo(flexman::logging::app, "Sorting solutions...\n");
        for (auto &pareto : results.pareto_fronts) {
            std::sort(pareto.solutions.begin(), pareto.solutions.end(), compare_ascending);
        }

        // Log results
        log_results(quire::info, results);

        // Save results
        save_results(search, results, parameters, modes, parser.getOption<std::string>("--output"));

        // Apply PSO if requested
        if (parser.getOption<bool>("--pso")) {
            qinfo(flexman::logging::app, "Running PSO optimization...\n");
            flexman::pso::SolverParameters solver_params{
                .num_particles  = parser.getOption<unsigned>("-pn"),
                .max_iterations = parser.getOption<unsigned>("-pm"),
                .inertia        = parser.getOption<double>("-pi"),
                .cognitive      = parser.getOption<double>("-pc"),
                .social         = parser.getOption<double>("-ps"),
            };
            auto optimized = flexman::pso::optimize_result(&search, solver_params, modes, results);
            log_results(quire::info, optimized);
            compare_results(results, optimized);
        }

        // Plot results
        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting solutions...\n");
            plot_pareto_front(results);
            plot_cost_breakdown(results);
            plot_machine_utilization(results);
        }
    } else if (parser.getOption<unsigned>("--run") == run_simulation) {
        // Run simulation for each machine mode
        std::vector<simulation_t> simulations;
        simulations.reserve(modes.size());

        auto simulation_steps = static_cast<unsigned>(search.time_max / search.time_delta);

        qinfo(flexman::logging::app, "Simulating individual machine operations...\n");
        for (const auto &mode : modes) {
            simulations.emplace_back(simulation_t{
                .data = flexman::simulation::simulate_single_mode(&search, mode, simulation_steps),
                .name = mode.machine_name,
            });
        }

        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting simulations...\n");
            plot_simulations(simulations);
        }
    }

    return 0;
}

/// @brief Execute continuous mode search and simulation.
auto execute_in_continuous_mode(cmdlp::Parser &parser) -> int
{
    // Similar to discrete mode but with continuous system
    continuous_search_t search;
    search.initial_state = {
        parser.getOption<double>("--initial_surface_roughness"),
        parser.getOption<double>("--initial_hardness"),
        parser.getOption<double>("--initial_temperature"),
        parser.getOption<double>("--initial_dimensional_accuracy"),
        parser.getOption<double>("--initial_coating_thickness"),
        parser.getOption<double>("--initial_stress_level")};
    search.target_state = {
        parser.getOption<double>("--target_surface_roughness"),
        parser.getOption<double>("--target_hardness"),
        parser.getOption<double>("--target_temperature"),
        parser.getOption<double>("--target_dimensional_accuracy"),
        parser.getOption<double>("--target_coating_thickness"),
        parser.getOption<double>("--target_stress_level")};
    search.time_max          = parser.getOption<double>("--time_max");
    search.time_delta        = parser.getOption<double>("--time_delta");
    search.threshold         = parser.getOption<double>("--threshold");
    search.timeout           = parser.getOption<double>("--timeout");
    search.interactive       = parser.getOption<bool>("--interactive");
    search.coarsening_factor = parser.getOption<unsigned>("--coarsening_factor");

    auto algorithm  = parser.getOption<unsigned>("-a");
    auto iterations = parser.getOption<unsigned>("--iterations");

    auto modes = create_machine_modes<continuous_mode_t>();
    std::vector<parameters_t> parameters;

    qinfo(flexman::logging::app, "Created %u continuous machine modes.\n", modes.size());

    if (parser.getOption<unsigned>("--run") == run_search) {
        result_t results;

        qinfo(flexman::logging::app, "Searching for optimal manufacturing sequences...\n");
        if (algorithm == algorithm_heuristic) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Heuristic>(
                &search, modes, iterations);
        } else if (algorithm == algorithm_exhaustive) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Exhaustive>(
                &search, modes, iterations);
        } else if (algorithm == algorithm_single_machine) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::SingleMachine>(
                &search, modes, iterations);
        }

        for (auto &pareto : results.pareto_fronts) {
            std::sort(pareto.solutions.begin(), pareto.solutions.end(), compare_ascending);
        }

        log_results(quire::info, results);
        save_results(search, results, parameters, modes, parser.getOption<std::string>("--output"));

        if (parser.getOption<bool>("--pso")) {
            qinfo(flexman::logging::app, "Running PSO optimization...\n");
            flexman::pso::SolverParameters solver_params{
                .num_particles  = parser.getOption<unsigned>("-pn"),
                .max_iterations = parser.getOption<unsigned>("-pm"),
                .inertia        = parser.getOption<double>("-pi"),
                .cognitive      = parser.getOption<double>("-pc"),
                .social         = parser.getOption<double>("-ps"),
            };
            auto optimized = flexman::pso::optimize_result(&search, solver_params, modes, results);
            log_results(quire::info, optimized);
            compare_results(results, optimized);
        }

        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting solutions...\n");
            plot_pareto_front(results);
            plot_cost_breakdown(results);
            plot_machine_utilization(results);
        }
    } else if (parser.getOption<unsigned>("--run") == run_simulation) {
        std::vector<simulation_t> simulations;
        simulations.reserve(modes.size());

        auto simulation_steps = static_cast<unsigned>(search.time_max / search.time_delta);

        qinfo(flexman::logging::app, "Simulating individual machine operations...\n");
        for (const auto &mode : modes) {
            simulations.emplace_back(simulation_t{
                .data = flexman::simulation::simulate_single_mode(&search, mode, simulation_steps),
                .name = mode.machine_name,
            });
        }

        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting simulations...\n");
            plot_simulations(simulations);
        }
    }

    return 0;
}

} // namespace manufacturing

/// @brief Main function.
auto main(int argc, char *argv[]) -> int
{
    json::config::string_delimiter_character = '"';

    cmdlp::Parser parser(argc, argv);
    manufacturing::setup_option_parser(parser);
    parser.parseOptions();

    if ((argc == 1) || parser.getOption<bool>("-h")) {
        std::cout << parser.getHelp() << "\n";
        return 0;
    }

    // Configure logging
    quire::log_level log_level = static_cast<quire::log_level>(parser.getOption<unsigned>("-lg"));
    flexman::logging::solution.set_log_level(log_level);
    flexman::logging::common.set_log_level(log_level);
    flexman::logging::search.set_log_level(log_level);
    flexman::logging::round.set_log_level(log_level);
    flexman::logging::app.set_log_level(log_level);

    if (log_level == quire::log_level::debug) {
        flexman::logging::solution.configure(
            {quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location});
        flexman::logging::common.configure(
            {quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location});
        flexman::logging::search.configure(
            {quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location});
        flexman::logging::round.configure(
            {quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location});
        flexman::logging::app.configure(
            {quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location});
    }

    qinfo(flexman::logging::app, "Starting multi-machine manufacturing system optimization...\n");

    if (parser.getOption<unsigned>("-m") == manufacturing::mode_discrete) {
        return manufacturing::execute_in_discrete_mode(parser);
    }
    if (parser.getOption<unsigned>("-m") == manufacturing::mode_continuous) {
        return manufacturing::execute_in_continuous_mode(parser);
    }

    return 0;
}
