#include "algo/simulate.hpp"
#include "algo/erase.hpp"
#include "algo/super_reconciliation.hpp"
#include "util/containers.hpp"
#include "util/MultivaluedNumber.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <unordered_map>
#include <vector>

namespace po = boost::program_options;
namespace chrono = std::chrono;
using perf_clock = chrono::steady_clock;
using us = chrono::microseconds;
using json = nlohmann::json;

/**
 * Input/output structure for specifying which metrics have to be evaluated in
 * a simulation-evaluation step and to provide the results of this evaluation.
 */
struct EvaluationResults
{
    /**
     * DL-score, difference between the number of duplications and losses
     * between the reference and the reconciled trees.
     */
    unsigned dlscore = 0;

    /**
     * Duration of the reconciliation step in microseconds.
     */
    long duration = 0;

    /**
     * Whether to compute the DL-score.
     */
    bool needs_dlscore = false;

    /**
     * Whether to measure the reconciliation time.
     */
    bool needs_duration = false;
};

/**
 * Evaluate given metrics of the reconciliation algorithm with a simulated
 * input under the given set of simulation parameters.
 *
 * @param prng Pseudo-random number generator to use for the simulation.
 * @param results Input/output argument for the metrics.
 * @param params Simulation parameters.
 */
template<typename PRNG>
void evaluate(
    PRNG& prng,
    EvaluationResults& results,
    SimulationParams& params)
{
    // Simulate the evolution of a fixed-size synteny by performing random
    // speciations, duplications and losses
    auto reference_tree = simulate_evolution(prng, params);

    // Erase loss and internal synteny labelling information from the
    // reference tree to make the input for the reconciliation algorithm
    auto reconciled_tree = reference_tree;
    erase_tree(reconciled_tree, std::begin(reconciled_tree));

    chrono::time_point<perf_clock> start;
    chrono::time_point<perf_clock> end;

    if (results.needs_duration)
    {
        start = perf_clock::now();
    }

    super_reconciliation(reconciled_tree);

    if (results.needs_duration)
    {
        end = perf_clock::now();
        results.duration = chrono::duration_cast<us, long>(end - start)
            .count();
    }

    if (results.needs_dlscore)
    {
        auto ref_score = get_dl_score(reference_tree);
        auto rec_score = get_dl_score(reconciled_tree);

        if (ref_score < rec_score)
        {
            throw std::logic_error{
                "Unexpected non-parcimonious reconciliation!"};
        }

        results.dlscore = ref_score - rec_score;
    }
}

/**
 * All arguments that can be passed to the program.
 * See below for a description of each argument.
 */
struct Arguments
{
    std::string output;
    std::vector<std::string> metrics;
    unsigned sample_size;
    unsigned jobs;

    MultivaluedNumber<unsigned> synteny_size;
    MultivaluedNumber<unsigned> event_depth;
    MultivaluedNumber<double> duplication_probability;
    MultivaluedNumber<double> loss_probability;
    MultivaluedNumber<double> loss_length_rate;
};

/**
 * Read arguments passed to the program and produce the
 * help message if requested by the user.
 *
 * @param result Filled with arguments passed to the program or
 * appropriate default values.
 * @param argc Number of arguments in argv.
 * @param argv Tokenized list of arguments passed to the program.
 * @return True if the program may continue, or false if it has
 * to be stopped.
 */
bool read_arguments(Arguments& result, int argc, const char* argv[])
{
    po::options_description root;

    po::options_description req_group{"Required arguments"};
    req_group.add_options()
        ("output,o",
         po::value(&result.output)
            ->value_name("PATH")
            ->required(),
         "path in which to create the output file")

        ("metrics,m",
         po::value(&result.metrics)
            ->value_name("METRIC")
            ->required(),
         "the metrics to evaluate, either 'dlscore' or 'duration'")
    ;
    root.add(req_group);

    po::options_description gen_opt_group{"General options"};
    gen_opt_group.add_options()
        ("help,h", "show this help message")

        ("sample-size,S",
         po::value(&result.sample_size)
            ->value_name("SIZE")
            ->default_value(1),
         "number of samples to take for each set of parameters")

        ("jobs,j",
         po::value(&result.jobs)
            ->value_name("JOBS")
            ->default_value(0),
         "number of threads to use for computing. If 0, automatically "
         "evaluate the best amount of threads based on the resources "
         "of the machine. Set to 1 to disable multithreading")
    ;
    root.add(gen_opt_group);

    po::options_description sim_opt_group{"Simulation parameters (accept "
        "either a single value, a set of values '{1, 2, 3}'\nor a range "
        "of values '[1:100]' with an optional step argument '[1:100:10]')"};
    sim_opt_group.add_options()
        ("synteny-size,s",
         po::value(&result.synteny_size)
            ->value_name("SIZE")
            ->default_value(5),
         "size of the ancestral synteny to evolve from")

        ("depth,d",
         po::value(&result.event_depth)
            ->value_name("SIZE")
            ->default_value(5),
         "maximum depth of events on a branch, not counting losses")

        ("p-dup,D",
         po::value(&result.duplication_probability)
            ->value_name("PROB")
            ->default_value(0.5),
         "probability for any given internal node to be a duplication")

        ("p-loss,L",
         po::value(&result.loss_probability)
            ->value_name("PROB")
            ->default_value(0.5),
         "probability for a loss under any given speciation node")

        ("p-length,R",
         po::value(&result.loss_length_rate)
            ->value_name("PROB")
            ->default_value(0.5),
         "parameter defining the geometric distribution of loss "
         "segments’ lengths")
    ;
    root.add(sim_opt_group);

    po::positional_options_description pos;
    pos.add("output", 1);

    po::variables_map values;
    po::store(
        po::command_line_parser(argc, argv)
            .options(root)
            .positional(pos)
            .run(),
        values);

    if (values.count("help"))
    {
        std::cout << "Usage: " << argv[0] << " output -m METRIC [options...]\n";
        std::cout << "\nEvaluate metrics of a sample of evolutions simulated"
            " for each\ngiven set of parameters.\n" << root;
        return false;
    }

    po::notify(values);
    return true;
}

/**
 * Lifecycle object that is private to each worker thread.
 * (derived from https://stackoverflow.com/a/10737658)
 */
struct ThreadLifecycle
{
    /**
     * Normal constructor for non-multithreaded scenarios.
     */
    ThreadLifecycle()
    {
        seed();
    }

    /**
     * This copy constructor gets called for each new worker thread
     * and serves as the initialization function.
     */
    ThreadLifecycle(const ThreadLifecycle&)
    {
        seed();
    }

    // Thread-local pseudo-random number generator
    std::mt19937 prng;

private:
    void seed()
    {
        std::random_device rd;
        this->prng.seed(rd());
    }
};

/**
 * Display progress on standard output.
 */
void report_progress(unsigned long performed, unsigned long total)
{
    if (performed != total && performed % 10 != 0)
    {
        return;
    }

    std::cout << std::fixed << std::setprecision(2) << "["
        << std::setw(6) << ((static_cast<double>(performed) / total) * 100)
        << "%] " << performed << "/" << total << " tasks performed"
        << std::endl;
}

int main(int argc, const char* argv[])
{
    using namespace std::string_literals;
    Arguments args;

    if (!read_arguments(args, argc, argv))
    {
        return EXIT_SUCCESS;
    }

    if (args.jobs > 0)
    {
        omp_set_num_threads(args.jobs);
    }

    ThreadLifecycle lifecycle;
    bool needs_dlscore = contains(args.metrics, "dlscore"s);
    bool needs_duration = contains(args.metrics, "duration"s);

    // Stores all the results for all samples of each set of parameters
    json results = json::array();

    // For each set of parameters, store the index in the results array
    // at which measurements are stored
    std::unordered_map<SimulationParams, std::size_t> find_params_index;

    // Track task progression
    unsigned long performed_tasks = 0;
    unsigned long total_tasks = args.sample_size
        * args.synteny_size.size()
        * args.event_depth.size()
        * args.duplication_probability.size()
        * args.loss_probability.size()
        * args.loss_length_rate.size();

    report_progress(performed_tasks, total_tasks);

    #pragma omp parallel for                                                   \
        firstprivate(                                                          \
            lifecycle, args, total_tasks,                                      \
            needs_dlscore, needs_duration)                                     \
        shared(                                                                \
            results, find_params_index, performed_tasks)                       \
        default(none)                                                          \
        collapse(6) schedule(dynamic)
    for (unsigned sample_id = 0; sample_id < args.sample_size; ++sample_id)
    {
    for (
        auto synteny_size = args.synteny_size.begin();
        synteny_size < args.synteny_size.end();
        ++synteny_size)
    {
    for (
        auto event_depth = args.event_depth.begin();
        event_depth < args.event_depth.end();
        ++event_depth)
    {
    for (
        auto duplication_prob = args.duplication_probability.begin();
        duplication_prob < args.duplication_probability.end();
        ++duplication_prob)
    {
    for (
        auto loss_prob = args.loss_probability.begin();
        loss_prob < args.loss_probability.end();
        ++loss_prob)
    {
    for (
        auto loss_length_rate = args.loss_length_rate.begin();
        loss_length_rate < args.loss_length_rate.end();
        ++loss_length_rate)
    {
        SimulationParams sample_params;
        sample_params.base_synteny = Synteny::generateDummy(*synteny_size);
        sample_params.event_depth = *event_depth;
        sample_params.duplication_probability = *duplication_prob;
        sample_params.loss_probability = *loss_prob;
        sample_params.loss_length_rate = *loss_length_rate;

        EvaluationResults sample_info;
        sample_info.needs_dlscore = needs_dlscore;
        sample_info.needs_duration = needs_duration;

        evaluate(lifecycle.prng, sample_info, sample_params);

        #pragma omp critical
        {
            // If no entry exist for the current set of params exist, create
            // one. Otherwise, directly append results to the existing one
            // by using the stored index
            if (!find_params_index.count(sample_params))
            {
                json sample_result = {
                    {"params", {
                        {"synteny_size", *synteny_size},
                        {"event_depth", *event_depth},
                        {"duplication_probability", *duplication_prob},
                        {"loss_probability", *loss_prob},
                        {"loss_length_rate", *loss_length_rate}
                    }}
                };

                if (needs_dlscore)
                {
                    sample_result["dlscore"] = json::array();
                }

                if (needs_duration)
                {
                    sample_result["duration"] = json::array();
                }

                find_params_index.emplace(sample_params, results.size());
                results.push_back(std::move(sample_result));
            }

            json& sample_result = results.at(
                find_params_index.at(sample_params));

            if (needs_dlscore)
            {
                sample_result["dlscore"].push_back(sample_info.dlscore);
            }

            if (needs_duration)
            {
                sample_result["duration"].push_back(sample_info.duration);
            }

            ++performed_tasks;
            report_progress(performed_tasks, total_tasks);
        }
    }}}}}}

    std::ofstream output(args.output);
    output << results;
    return EXIT_SUCCESS;
}
