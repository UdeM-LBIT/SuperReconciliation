#include "algo/simulate.hpp"
#include "algo/erase.hpp"
#include "algo/super_reconciliation.hpp"
#include "util/containers.hpp"
#include "util/MultivaluedNumber.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <nlohmann/json.hpp>
#include <ostream>
#include <fstream>
#include <vector>

namespace po = boost::program_options;
namespace chrono = std::chrono;
using perf_clock = chrono::steady_clock;
using us = chrono::microseconds;
using json = nlohmann::json;

struct EvaluationResults
{
    unsigned scoredif = 0;
    long duration = 0;

    bool needs_scoredif = false;
    bool needs_duration = false;
};

template<typename PRNG>
void evaluate(
    EvaluationResults& results,
    EvolutionParams<PRNG>& params)
{
    auto reference_tree = simulate_evolution(params);
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

    if (results.needs_scoredif)
    {
        auto ref_score = get_dl_score(reference_tree);
        auto rec_score = get_dl_score(reconciled_tree);

        if (ref_score < rec_score)
        {
            throw std::logic_error{
                "Unexpected non-parcimonious reconciliation!"};
        }

        results.scoredif = ref_score - rec_score;
    }
}

template<typename PRNG>
json sample(
    EvolutionParams<PRNG>& params,
    unsigned sample_size,
    bool needs_scoredif,
    bool needs_duration)
{
    json results = {
        {"params", {
            {"synteny_size", params.base_synteny.size()},
            {"event_depth", params.event_depth},
            {"duplication_probability", params.duplication_probability},
            {"loss_probability", params.loss_probability},
            {"loss_length_rate", params.loss_length_rate}
        }}
    };

    if (needs_scoredif)
    {
        results["scoredif"] = json::array();
    }

    if (needs_duration)
    {
        results["duration"] = json::array();
    }

    EvaluationResults sample_info;
    sample_info.needs_scoredif = needs_scoredif;
    sample_info.needs_duration = needs_duration;

    for (unsigned i = 0; i < sample_size; ++i)
    {
        evaluate(sample_info, params);

        if (needs_scoredif)
        {
            results["scoredif"].push_back(sample_info.scoredif);
        }

        if (needs_duration)
        {
            results["duration"].push_back(sample_info.duration);
        }
    }

    return results;
}

struct Arguments
{
    std::string output;
    std::vector<std::string> metrics;
    unsigned sample_size;
    unsigned long seed;
    MultivaluedNumber<unsigned> synteny_size = 5;
    MultivaluedNumber<unsigned> event_depth = 5;
    MultivaluedNumber<double> duplication_probability = 0.5;
    MultivaluedNumber<double> loss_probability = 0.5;
    MultivaluedNumber<double> loss_length_rate = 0.5;
};

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
         "the metrics to evaluate, either 'scoredif' or 'duration'")
    ;

    root.add(req_group);
    po::options_description gen_opt_group{"General options"};

    gen_opt_group.add_options()
        ("help,h", "show this help message")

        ("sample-size,S",
         po::value(&result.sample_size)
            ->value_name("SIZE")
            ->default_value(1),
         "number of samples to use for each set of parameters")

        ("seed",
         po::value(&result.seed)
            ->value_name("NUMBER")
            ->default_value(0),
         "seed to use for pseudo-random number generation. If 0, will "
         "use a system-provided random number")
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

int main(int argc, const char* argv[])
{
    using namespace std::string_literals;
    Arguments args;

    if (!read_arguments(args, argc, argv))
    {
        return EXIT_SUCCESS;
    }

    std::ofstream output(args.output);
    bool needs_scoredif = contains(args.metrics, "scoredif"s);
    bool needs_duration = contains(args.metrics, "duration"s);

    json result = json::array();
    unsigned long seed = args.seed;

    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }

    std::mt19937 prng{seed};
    EvolutionParams<std::mt19937> params;
    params.random_generator = &prng;

    for (auto synteny_size : args.synteny_size)
    {
        params.base_synteny = Synteny::generateDummy(synteny_size);
    for (auto event_depth : args.event_depth)
    {
        params.event_depth = event_depth;
    for (auto duplication_probability : args.duplication_probability)
    {
        params.duplication_probability = duplication_probability;
    for (auto loss_probability : args.loss_probability)
    {
        params.loss_probability = loss_probability;
    for (auto loss_length_rate : args.loss_length_rate)
    {
        params.loss_length_rate = loss_length_rate;
        result.push_back(sample(
            params, args.sample_size,
            needs_scoredif, needs_duration));
    }
    }
    }
    }
    }

    output << result;
    return EXIT_SUCCESS;
}
