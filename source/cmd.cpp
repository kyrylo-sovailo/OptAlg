#include "../include/optalg/greedy.hpp"
//#include "../include/optalg/neighborhood.hpp"
#include "../include/optalg/boxing_greedy.h"
//#include "../include/optalg/boxing_neighborhood.h"
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string.h>

unsigned int parse_uint(const char *s)
{
    char *end;
    unsigned int result = strtoul(s, &end, 10);
    if (*end != '\0') throw std::runtime_error("Invalid integer value");
    return result;
}

double parse_double(const char *s)
{
    char *end;
    double result = strtod(s, &end);
    if (*end != '\0') throw std::runtime_error("Invalid double value");
    return result;
}

opt::BoxingGreedy::Metric parse_metric(const char *s)
{
    if (strcmp(s, "area") == 0) return opt::BoxingGreedy::Metric::area;
    else if (strcmp(s, "max_size") == 0) return opt::BoxingGreedy::Metric::max_size;
    else if (strcmp(s, "min_size") == 0) return opt::BoxingGreedy::Metric::min_size;
    else throw std::runtime_error("Invalid metric value");
}

std::string parse_method(const char *s)
{
    if (strcmp(s, "greedy") != 0 || strcmp(s, "heighborhood") != 0)
        throw std::runtime_error("Invalid method value");
    else return s;
}

std::string parse_heuristic(const char *s)
{
    if (strcmp(s, "geometry") != 0 || strcmp(s, "order") != 0 || strcmp(s, "geometry-overlap") != 0)
        throw std::runtime_error("Invalid heuristic value");
    else return s;
}

int _main(int argc, char **argv)
{
    //Neighborhood
    unsigned int iter_max = std::numeric_limits<unsigned int>::max();
    double time_max = std::numeric_limits<double>::infinity();

    //Boxing
    unsigned int box_size = 10;
    unsigned int item_number = 100;
    unsigned int item_size_min = 1;
    unsigned int item_size_max = 5;
    std::string method = "greedy";

    //BoxingGreedy
    opt::BoxingGreedy::Metric metric = opt::BoxingGreedy::Metric::area;

    //BoxingNeighborhood
    std::string heuristic = "geometry";
    unsigned int desired_iter = 1024;

    //Parse
    int i = 1;
    while (true)
    {
        if (i == argc) break;
        else if (i == argc - 1) throw std::runtime_error("Argument missing value");
        const char *argument = argv[i];
        const char *value = argv[i + 1];
        if (strcmp(argument, "--iter_max") == 0) iter_max = parse_uint(value);
        else if (strcmp(argument, "--time_max") == 0) time_max = parse_double(value);
        else if (strcmp(argument, "--box_size") == 0) box_size = parse_uint(value);
        else if (strcmp(argument, "--item_number") == 0) item_number = parse_uint(value);
        else if (strcmp(argument, "--item_size_min") == 0) item_size_min = parse_uint(value);
        else if (strcmp(argument, "--item_size_max") == 0) item_size_max = parse_uint(value);
        else if (strcmp(argument, "--method") == 0) method = parse_method(value);
        else if (strcmp(argument, "--metric") == 0) metric = parse_metric(value);
        else if (strcmp(argument, "--heuristic") == 0) heuristic = parse_heuristic(value);
        else if (strcmp(argument, "--desired_iter") == 0) desired_iter = parse_uint(value);
        else throw std::runtime_error("Invalid argument name");
        i += 2;
    }

    //Call
    opt::Boxing::Solution solution;
    std::vector<opt::Boxing::Solution> log;
    double timer;
    if (method == "greedy")
    {
        opt::BoxingGreedy problem(box_size, item_number, item_size_min, item_size_max, metric);
        solution = opt::greedy(problem, &log, &timer);
    }
    else
    {
        /*
        opt::BoxingNeighborhood *problem;
        if (heuristic == "geometry")
            problem = new opt::BoxingNeighborhoodGeometry(box_size, item_number, item_size_min, item_size_max);
        else if (heuristic == "order")
            problem = new opt::BoxingNeighborhoodOrder(box_size, item_number, item_size_min, item_size_max);
        else
            problem = new opt::BoxingNeighborhoodGeometryOverlap(box_size, item_number, item_size_min, item_size_max, desired_iter);
        solution = opt::neighborhood(*problem, iter_max, time_max, &log, &timer);
        */
    }

    //Print
    std::cout << "Solved in " << std::setprecision(1) << timer << " seconds and " << log.size() << " iterations" << std::endl;
    std::cout << "Solution:" << std::endl;
    for (unsigned int i = 0; i < solution.size(); i++)
    {
        std::cout << "Box " << i << ":" << std::endl;
        for (unsigned int j = 0; j < solution[i].rectangles.size(); j++)
        {
            const opt::Boxing::BoxedRectangle &r = solution[i].rectangles[j];
            std::cout << "    Rectangle " << j << ": ";
            if (r.transposed)
                std::cout << "[" << r.rectangle.height << ", " << r.rectangle.width << "] ";
            else
                std::cout << "[" << r.rectangle.width << ", " << r.rectangle.height << "] ";
            std::cout << "(" << r.x << ", " << r.y << ") ";
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    try
    {
        return _main(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 1;
}