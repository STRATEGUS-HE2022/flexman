/// @file main.cpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief

#include "fsmlib_support.hpp"
#include "plotting.hpp"
#include "defines.hpp"
#include "builder.hpp"
#include "search.hpp"

#include <flexman/simulation/simulate.hpp>
#include <flexman/serialization.hpp>
#include <flexman/pso/optimize.hpp>
#include <cmdlp/parser.hpp>
#include <cmath>

namespace tapping
{

enum run_option {
    run_search,
    run_simulation
};

enum mode_option {
    mode_discrete,
    mode_continuous
};

enum algorithm_type {
    algorithm_heuristic,
    algorithm_exhaustive,
    algorithm_single_machine
};

/// @brief Compares two solutions for ascending order based on energy and time.
/// @param lhs The first solution to compare.
/// @param rhs The second solution to compare.
/// @return true if lhs comes before rhs in ascending order, otherwise false.
inline bool compare_ascending(const solution_t &lhs, const solution_t &rhs)
{
    // Primary sort: by energy (ascending).
    if (!fsmlib::feq::approximately_equal(lhs.resources.energy, rhs.resources.energy)) {
        return lhs.resources.energy < rhs.resources.energy;
    }
    // Secondary sort: by time (ascending).
    return lhs.resources.time < rhs.resources.time;
}

/// @brief Logs the details of each Pareto front and its solutions.
/// @param manager The search manager.
/// @param results The result set.
inline void log_results(quire::log_level log_level, const tapping::result_t &results)
{
    qlog(flexman::logging::app, log_level, "============================================================\n");
    for (const auto &pareto : results.pareto_fronts) {
        // Log the Pareto front metadata (step length and runtime).
        qlog(flexman::logging::app, log_level, "Pareto front (step: %8.3f s, runtime: %8.3f s):\n", pareto.step_length, pareto.runtime);
        // Log each solution in the Pareto front.
        for (const auto &solution : pareto.solutions) {
            qlog(flexman::logging::app, log_level, "\t%s\n", flexman::to_string(solution).c_str());
        }
    }
    qlog(flexman::logging::app, log_level, "============================================================\n");
}

/// @brief Generates num points between start and max and return as vector.
/// @tparam T The type of the vector.
/// @param start The minimum value.
/// @param stop The maximum value.
/// @param num The number of elements.
/// @return The generated vector.
template <typename T>
[[nodiscard]] inline auto linspace(T start, T stop, unsigned num = 100)
{
    std::vector<T> result(num, 1);
    if (num == 0) {
        return result;
    }
    bool are_the_same;
    if constexpr (std::is_floating_point<T>::value) {
        are_the_same = fsmlib::feq::approximately_equal(start, stop);
    } else {
        are_the_same = start == stop;
    }
    if (num == 1) {
        result[0] = stop;
        return result;
    }
    if (are_the_same) {
        for (unsigned i = 0; i < num; ++i) {
            result[i] = stop;
        }
    } else if (start < stop) {
        T step = (stop - start) / static_cast<T>(num - 1);
        for (unsigned i = 0; i < num; ++i) {
            result[i] = start + step * static_cast<T>(i);
        }
    } else {
        // When start > stop, generate a decreasing sequence
        T step = (start - stop) / static_cast<T>(num - 1);
        for (unsigned i = 0; i < num; ++i) {
            result[i] = start - step * static_cast<T>(i);
        }
    }
    return result;
}

/// @brief Determines the comparison result as a string with color coding.
/// @param value1 The initial value.
/// @param value2 The new value.
/// @return A string indicating "improved," "worsened," or "unchanged," with appropriate color coding.
std::string compare_state(double value1, double value2)
{
    if (value1 > value2) {
        return std::string(quire::ansi::fg::bright_green) + "improved " + quire::ansi::util::reset;
    } else if (value1 < value2) {
        return std::string(quire::ansi::fg::bright_red) + "worsened " + quire::ansi::util::reset;
    } else {
        return std::string(quire::ansi::util::reset) + "unchanged" + quire::ansi::util::reset;
    }
}

template <typename State, typename Resources>
void compare_results(const flexman::Result<State, Resources> &result1,
                     const flexman::Result<State, Resources> &result2)
{
    std::string improved  = std::string(quire::ansi::fg::bright_green) + "improved" + quire::ansi::util::reset;
    std::string worsened  = std::string(quire::ansi::fg::bright_red) + "worsened" + quire::ansi::util::reset;
    std::string unchanged = std::string(quire::ansi::util::reset) + "unchanged" + quire::ansi::util::reset;

    // Compare the number of Pareto fronts.
    if (result1.pareto_fronts.size() != result2.pareto_fronts.size()) {
        qwarning(
            flexman::logging::app,
            "Results differ in the number of Pareto fronts (%u vs %u)\n.",
            result1.pareto_fronts.size(),
            result2.pareto_fronts.size());
        return;
    }
    // Compare each Pareto front.
    for (size_t i = 0; i < result1.pareto_fronts.size(); ++i) {
        const auto &front1 = result1.pareto_fronts[i];
        const auto &front2 = result2.pareto_fronts[i];

        // Compare the number of solutions in each Pareto front
        if (front1.solutions.size() != front2.solutions.size()) {
            qwarning(
                flexman::logging::app,
                "Pareto front %u differ in the number of solutions (%u vs %u)\n.",
                i + 1,
                front1.solutions.size(),
                front2.solutions.size());
            continue;
        }

        // Compare each solution in the Pareto front
        for (size_t j = 0; j < front1.solutions.size(); ++j) {
            const auto &sol1 = front1.solutions[j];
            const auto &sol2 = front2.solutions[j];

            std::string time_comparison   = compare_state(sol1.resources.time, sol2.resources.time);
            std::string energy_comparison = compare_state(sol1.resources.energy, sol2.resources.energy);

            qinfo(
                flexman::logging::app,
                "Solution %2u in Pareto Front %2u, it's %s in time (%8.4f -> %8.4f), and it's %s in energy (%8.4f -> %8.4f).\n",
                j + 1,
                i + 1,
                time_comparison.c_str(),
                sol1.resources.time,
                sol2.resources.time,
                energy_comparison.c_str(),
                sol1.resources.energy,
                sol2.resources.energy);
        }
    }
}

template <typename SearchManager, typename Parameter, typename Mode>
inline void save_results(
    const SearchManager &manager,
    const tapping::result_t &results,
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
    if (!json::parser::write_file(filename, root, true, 4u)) {
        std::cerr << "Failed to save to `" << filename << "`.\n";
    }
}

void setup_option_parser(cmdlp::Parser &parser)
{
    // Add the help.
    parser.addToggle("-h", "--help", "Show this help.", false);
    // Select the run mode.
    parser.addMultiOption(
        "-r", "--run", "Run (0) search, (1) simulation.",
        {
            std::to_string(run_search),
            std::to_string(run_simulation),
        },
        std::to_string(run_search));
    // Select the execution mode.
    parser.addMultiOption(
        "-m", "--mode", "Run (0) discrete, (1) continuous.",
        {
            std::to_string(mode_discrete),
            std::to_string(mode_continuous),
        },
        std::to_string(mode_discrete));
    // Select the algorithm.
    parser.addMultiOption(
        "-a", "--algorithm", "Run (0) heuristic, (1) exhaustive, (2) single machine.",
        {
            std::to_string(algorithm_heuristic),
            std::to_string(algorithm_exhaustive),
            std::to_string(algorithm_single_machine),
        },
        std::to_string(algorithm_heuristic));
    // Post-search optimization.
    parser.addToggle("-p", "--pso", "Enable post-search optimization using PSO", false);
    parser.addOption("-pn", "--pso_num_particles", "Number of particles in the PSO swarm", 100, false);
    parser.addOption("-pm", "--pso_max_iterations", "Maximum number of iterations for PSO", 50, false);
    parser.addOption("-pi", "--pso_inertia", "Inertia weight for PSO (controls exploration vs exploitation)", .2, false);
    parser.addOption("-pc", "--pso_cognitive", "Cognitive weight for PSO (influence of personal best)", .4, false);
    parser.addOption("-ps", "--pso_social", "Social weight for PSO (influence of global best)", .4, false);
    // Set the output file.
    parser.addOption("-o", "--output", "The file where the execution results are saved", "output.json", false);
    // Search parameters.
    parser.addOption("-dp", "--depth", "The target tapping depth", 40.0, false);
    parser.addOption("-tm", "--time_max", "The maximum simulated time", 120.0, false);
    parser.addOption("-td", "--time_delta", "The time delta", 0.01, false);
    parser.addOption("-th", "--threshold", "Used to determine when a solution is considered complete", 0.01, false);
    parser.addOption("-dl", "--timeout", "For how long is the algorithm supposed to run approximately", 120.0, false);
    parser.addToggle("-in", "--interactive", "Enable the interactive mode", false);
    // Search manager parameters.
    parser.addOption("-it", "--iterations", "The number of iterations for the search", 12u, false);
    // Gear factors parameters.
    parser.addOption("-gu", "--min_gear", "The minimum gear range", 5u, false);
    parser.addOption("-gl", "--max_gear", "The maximum gear range", 50u, false);
    parser.addOption("-gn", "--num_gear", "The minimum gear range", 8u, false);
    // The log level.
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
    // Enable plot.
    parser.addToggle("-pl", "--plot", "Plot the results", false);
}

int execute_in_discrete_mode(cmdlp::Parser &parser)
{
    // Search parameters.
    tapping::discrete_search_t search;
    search.initial_state = { 0, 0, 0 };
    search.target_state  = { 0, 0, parser.getOption<double>("--depth") };
    search.time_max      = parser.getOption<double>("--time_max");
    search.time_delta    = parser.getOption<double>("--time_delta");
    search.threshold     = parser.getOption<double>("--threshold");
    search.timeout       = parser.getOption<double>("--timeout");
    search.interactive   = parser.getOption<bool>("--interactive");

    // Select the algorithm.
    unsigned algorithm = parser.getOption<unsigned>("-a");

    // Get the number of iterations.
    unsigned iterations = parser.getOption<unsigned>("--iterations");

    // Create the gear factors.
    const auto gear_factors = tapping::linspace<double>(
        parser.getOption<unsigned>("--max_gear"),
        parser.getOption<unsigned>("--min_gear"),
        parser.getOption<unsigned>("--num_gear"));

    // Vector of model builders.
    std::vector<tapping::parameters_t> parameters;
    // Vector of modes.
    std::vector<tapping::discrete_mode_t> modes;
    // The standard tapping parameters.
    tapping::parameters_t base_parameters;
    for (flexman::mode_id_t i = 0; i < gear_factors.size(); ++i) {
        base_parameters.Gr = gear_factors[i];
        // Save a copy of the tapping parameters.
        parameters.emplace_back(base_parameters);
        // Generate the mode.
        modes.emplace_back(tapping::builder_t(base_parameters).make_discrete_mode(i, search.time_delta));
    }

    // Run the search.
    if (parser.getOption<unsigned>("--run") == run_search) {
        tapping::result_t results;

        qinfo(flexman::logging::app, "Searching...\n");
        if (algorithm == algorithm_heuristic) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Heuristic>(&search, modes, iterations);
        } else if (algorithm == algorithm_exhaustive) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Exhaustive>(&search, modes, iterations);
        } else if (algorithm == algorithm_single_machine) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::SingleMachine>(&search, modes, iterations);
        }

        // Sort the results.
        qinfo(flexman::logging::app, "Sorting solutions...\n");
        for (auto &pareto : results.pareto_fronts) {
            std::sort(pareto.solutions.begin(), pareto.solutions.end(), tapping::compare_ascending);
        }

        // Log the results.
        tapping::log_results(quire::info, results);

        // Save results.
        tapping::save_results(
            search,
            results,
            parameters,
            modes,
            parser.getOption<std::string>("--output"));

        // Apply PSO if requested.
        if (parser.getOption<bool>("--pso")) {
            qinfo(flexman::logging::app, "Running PSO...\n");
            flexman::pso::SolverParameters solver_params{
                .num_particles  = parser.getOption<unsigned>("-pn"),
                .max_iterations = parser.getOption<unsigned>("-pm"),
                .inertia        = parser.getOption<double>("-pi"),
                .cognitive      = parser.getOption<double>("-pc"),
                .social         = parser.getOption<double>("-ps"),
            };
            auto optimized = flexman::pso::optimize_result(&search, solver_params, modes, results);
            // Log the results.
            tapping::log_results(quire::info, optimized);
            // Compare the results.
            compare_results(results, optimized);
        }

        // Plot the results.
        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting solutions...\n");
            tapping::plot_pareto_front(results);
        }
    }
    // Run the simulation.
    else if (parser.getOption<unsigned>("--run") == run_simulation) {
        // Vector of solutions.
        std::vector<tapping::simulation_t> simulations;

        // Number of simulation steps.
        unsigned simulation_steps = static_cast<unsigned>(search.time_max / search.time_delta);

        // Run the simulation.
        qinfo(flexman::logging::app, "Simulating...\n");
        for (const auto &mode : modes) {
            simulations.emplace_back(
                tapping::simulation_t{
                    .data = flexman::simulation::simulate_single_mode(&search, mode, simulation_steps),
                    .name = "Mode " + std::to_string(mode.id),
                });
        }

        // Plot the results.
        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting...\n");
            tapping::plot_simulations(simulations);
        }
    }

    return 0;
}

int execute_in_continuous_mode(cmdlp::Parser &parser)
{
    // Search parameters.
    tapping::continuous_search_t search;
    search.initial_state = { 0, 0, 0 };
    search.target_state  = { 0, 0, parser.getOption<double>("--depth") };
    search.time_max      = parser.getOption<double>("--time_max");
    search.time_delta    = parser.getOption<double>("--time_delta");
    search.threshold     = parser.getOption<double>("--threshold");
    search.timeout       = parser.getOption<double>("--timeout");
    search.interactive   = parser.getOption<bool>("--interactive");

    // Select the algorithm.
    unsigned algorithm = parser.getOption<unsigned>("-a");

    // Get the number of iterations.
    unsigned iterations = parser.getOption<unsigned>("--iterations");

    // Create the gear factors.
    const auto gear_factors = tapping::linspace<double>(
        parser.getOption<unsigned>("--max_gear"),
        parser.getOption<unsigned>("--min_gear"),
        parser.getOption<unsigned>("--num_gear"));

    // Vector of model builders.
    std::vector<tapping::parameters_t> parameters;
    // Vector of modes.
    std::vector<tapping::continous_mode_t> modes;
    // The standard tapping parameters.
    tapping::parameters_t base_parameters;
    for (flexman::mode_id_t i = 0; i < gear_factors.size(); ++i) {
        base_parameters.Gr = gear_factors[i];
        // Save a copy of the tapping parameters.
        parameters.emplace_back(base_parameters);
        // Generate the mode.
        modes.emplace_back(tapping::builder_t(base_parameters).make_continuous_mode(i));
    }

    // Run the search.
    if (parser.getOption<unsigned>("--run") == run_search) {
        tapping::result_t results;

        qinfo(flexman::logging::app, "Searching...\n");
        if (algorithm == algorithm_heuristic) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Heuristic>(&search, modes, iterations);
        } else if (algorithm == algorithm_exhaustive) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::Exhaustive>(&search, modes, iterations);
        } else if (algorithm == algorithm_single_machine) {
            results = flexman::search::perform_search<flexman::search::SearchAlgorithm::SingleMachine>(&search, modes, iterations);
        }

        // Sort the results.
        qinfo(flexman::logging::app, "Sorting solutions...\n");
        for (auto &pareto : results.pareto_fronts) {
            std::sort(pareto.solutions.begin(), pareto.solutions.end(), tapping::compare_ascending);
        }

        // Log the results.
        tapping::log_results(quire::info, results);

        // Save the results.
        tapping::save_results(
            search,
            results,
            parameters,
            modes,
            parser.getOption<std::string>("--output"));

        // Apply PSO if requested.
        if (parser.getOption<bool>("--pso")) {
            qinfo(flexman::logging::app, "Running PSO...\n");
            flexman::pso::SolverParameters solver_params{
                .num_particles  = parser.getOption<unsigned>("-pn"),
                .max_iterations = parser.getOption<unsigned>("-pm"),
                .inertia        = parser.getOption<double>("-pi"),
                .cognitive      = parser.getOption<double>("-pc"),
                .social         = parser.getOption<double>("-ps"),
            };
            auto optimized = flexman::pso::optimize_result(&search, solver_params, modes, results);
            // Log the results.
            tapping::log_results(quire::info, optimized);
            // Compare the results.
            compare_results(results, optimized);
        }

        // Plot the results.
        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting solutions...\n");
            tapping::plot_pareto_front(results);
        }
    }
    // Run the simulation.
    else if (parser.getOption<unsigned>("--run") == run_simulation) {
        // Vector of solutions.
        std::vector<tapping::simulation_t> simulations;

        // Number of simulation steps.
        unsigned simulation_steps = static_cast<unsigned>(search.time_max / search.time_delta);

        // Run the simulation.
        qinfo(flexman::logging::app, "Simulating...\n");
        for (const auto &mode : modes) {
            simulations.emplace_back(flexman::simulation::simulate_single_mode(&search, mode, simulation_steps));
        }

        // Plot the results.
        if (parser.getOption<bool>("--plot")) {
            qinfo(flexman::logging::app, "Plotting...\n");
            tapping::plot_simulations(simulations);
        }
    }

    return 0;
}

} // namespace tapping

int main(int argc, char *argv[])
{
    json::config::string_delimiter_character = '"';

    cmdlp::Parser parser(argc, argv);

    tapping::setup_option_parser(parser);

    parser.parseOptions();

    // If no option was provided, or help was requested.
    if ((argc == 1) || parser.getOption<bool>("-h")) {
        std::cout << parser.getHelp() << "\n";
        return 0;
    }

    quire::log_level log_level = static_cast<quire::log_level>(parser.getOption<unsigned>("-lg"));
    flexman::logging::solution.set_log_level(log_level);
    flexman::logging::common.set_log_level(log_level);
    flexman::logging::search.set_log_level(log_level);
    flexman::logging::round.set_log_level(log_level);
    flexman::logging::app.set_log_level(log_level);
    if (log_level == quire::log_level::debug) {
        flexman::logging::solution.configure({ quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location });
        flexman::logging::common.configure({ quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location });
        flexman::logging::search.configure({ quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location });
        flexman::logging::round.configure({ quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location });
        flexman::logging::app.configure({ quire::option_t::time, quire::option_t::header, quire::option_t::level, quire::option_t::location });
    }

    if (parser.getOption<unsigned>("-m") == tapping::mode_discrete) {
        return tapping::execute_in_discrete_mode(parser);
    }
    if (parser.getOption<unsigned>("-m") == tapping::mode_continuous) {
        return tapping::execute_in_continuous_mode(parser);
    }
    return 0;
}
