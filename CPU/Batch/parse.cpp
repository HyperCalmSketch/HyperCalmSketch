#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "../datasets/trace_utils.h"

#include "params.h"

using namespace std;
using namespace boost::program_options;

void ParseArgs(int argc, char** argv) {
    options_description opts("Options");
    opts.add_options()
        ("fileName,f", value<string>()->required(), "file name")
        ("sketchName,s", value<int>()->required(), "sketch name")
        ("checkName,s", value<int>()->required(), "check name")
        ("time,t", value<int>()->required(), "repeat time")
        ("topk,k", value<int>()->required(), "topk, unused")
        ("batch_size,l", value<int>()->required(), "batch size threshold")
        ("memory,m", value<int>()->required(), "memory")
        ("batch_time,b", value<double>()->required(), "batch time")
        ("unit_time,u", value<double>()->required(),"unit time");
    variables_map vm;

    store(parse_command_line(argc, argv, opts), vm);
    if (vm.count("fileName")) {
        fileName = vm["fileName"].as<string>();
    } else {
        printf("please use -f to specify the path of dataset.\n");
        exit(0);
    }
    if (vm.count("sketchName")) {
        sketchName = vm["sketchName"].as<int>();
        if (sketchName < 1 || sketchName > 4) {
            printf("sketchName < 1 || sketchName > 4\n");
            exit(0);
        }
    } else {
        printf("please use -s to specify the name of sketch.\n");
        exit(0);
    }
    if (vm.count("checkName")) {
        sketchName = vm["checkName"].as<int>();
        if (sketchName < 1 || sketchName > 3) {
            printf("checkName < 1 || checkName > 4\n");
            exit(0);
        }
    } else {
        printf("please use -c to specify the type of test.\n");
        exit(0);
    }
    if (vm.count("time"))
        repeat_time = vm["time"].as<int>();
    if (vm.count("batch_size"))
        BATCH_SIZE_LIMIT = vm["batch_size"].as<int>();
    if (vm.count("memory"))
        memory = vm["memory"].as<int>();
    if (vm.count("batch_time"))
        BATCH_TIME = vm["batch_time"].as<double>();
    if (vm.count("unit_time"))
        UNIT_TIME = vm["unit_time"].as<double>();
}
