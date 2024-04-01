#include "../include/optalg/greedy.hpp"
#include "../include/optalg/neighborhood.hpp"
#include "../include/optalg/boxing_greedy.h"
#include "../include/optalg/boxing_neighborhood.h"
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string.h>

bool parse_bool(const char *s)
{
    if (strcmp(s, "true") == 0) return true;
    else if (strcmp(s, "false") == 0) return false;
    else throw std::runtime_error("Invalid boolean value");
}

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
    if (strcmp(s, "greedy") != 0 && strcmp(s, "neighborhood") != 0)
        throw std::runtime_error("Invalid method value");
    else return s;
}

std::string parse_neighborhood(const char *s)
{
    if (strcmp(s, "geometry") != 0 && strcmp(s, "order") != 0 && strcmp(s, "geometry-overlap") != 0)
        throw std::runtime_error("Invalid heuristic value");
    else return s;
}

int _main(int argc, char **argv)
{
    //Mode
    std::string method = "greedy";
    opt::BoxingGreedy::Metric metric = opt::BoxingGreedy::Metric::area;
    std::string neighborhood = "geometry";
    unsigned int loglevel = 1;

    //Problem
    unsigned int box_size = 10;
    unsigned int item_number = 100;
    unsigned int item_size_min = 1;
    unsigned int item_size_max = 5;
    unsigned int seed = 0;
    unsigned int window = 1;
    unsigned int hwindow = 0;
    unsigned int desired_iter = 100;

    //Solution
    unsigned int iter_max = std::numeric_limits<unsigned int>::max();
    double time_max = std::numeric_limits<double>::infinity();
    bool return_good = true;
    
    //Parse
    for (int i = 1;;)
    {
        if (i == argc) break;
        else if (i == argc - 1) throw std::runtime_error("Argument missing value");
        const char *argument = argv[i];
        const char *value = argv[i + 1];
        if (strcmp(argument, "--method") == 0) method = parse_method(value);
        else if (strcmp(argument, "--metric") == 0) metric = parse_metric(value);
        else if (strcmp(argument, "--neighborhood") == 0) neighborhood = parse_neighborhood(value);
        else if (strcmp(argument, "--loglevel") == 0) loglevel = parse_uint(value);

        else if (strcmp(argument, "--box_size") == 0) box_size = parse_uint(value);
        else if (strcmp(argument, "--item_number") == 0) item_number = parse_uint(value);
        else if (strcmp(argument, "--item_size_min") == 0) item_size_min = parse_uint(value);
        else if (strcmp(argument, "--item_size_max") == 0) item_size_max = parse_uint(value);
        else if (strcmp(argument, "--seed") == 0) seed = parse_uint(value);
        else if (strcmp(argument, "--window") == 0) window = parse_uint(value);
        else if (strcmp(argument, "--hwindow") == 0) hwindow = parse_uint(value);
        else if (strcmp(argument, "--desired_iter") == 0) desired_iter = parse_uint(value);
        
        else if (strcmp(argument, "--iter_max") == 0) iter_max = parse_uint(value);
        else if (strcmp(argument, "--time_max") == 0) time_max = parse_double(value);
        else if (strcmp(argument, "--return_good") == 0) return_good = parse_bool(value);
        else throw std::runtime_error("Invalid argument name");
        i += 2;
    }

    //Call
    std::unique_ptr<opt::Boxing> boxing;
    std::vector<opt::Boxing::Box> boxes;
    unsigned int iteration_count;
    double timer;
    if (method == "greedy")
    {
        typedef opt::BoxingGreedy Problem;
        Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, metric);
        boxing.reset(problem);
        std::vector<Problem::Solution> log;
        Problem::Solution solution = opt::greedy(*problem, &log, &timer);
        boxes = problem->get_boxes(solution);
        iteration_count = log.size() - 1;
    }
    else if (neighborhood == "geometry")
    {
        typedef opt::BoxingNeighborhoodGeometry Problem;
        Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, window, hwindow);
        boxing.reset(problem);
        std::vector<Problem::Solution> log;
        Problem::Solution solution = opt::neighborhood(*problem, iter_max, time_max, return_good, &log, &timer);
        boxes = problem->get_boxes(solution);
        iteration_count = log.size() - 1;
    }
    else if (neighborhood == "order")
    {
        typedef opt::BoxingNeighborhoodOrder Problem;
        Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, window);
        boxing.reset(problem);
        std::vector<Problem::Solution> log;
        Problem::Solution solution = opt::neighborhood(*problem, iter_max, time_max, return_good, &log, &timer);
        boxes = problem->get_boxes(solution);
        iteration_count = log.size() - 1;
    }
    else
    {
        typedef opt::BoxingNeighborhoodGeometryOverlap Problem;
        Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, window, hwindow, desired_iter);
        boxing.reset(problem);
        std::vector<Problem::Solution> log;
        Problem::Solution solution = opt::neighborhood(*problem, iter_max, time_max, return_good, &log, &timer);
        boxes = problem->get_boxes(solution);
        iteration_count = log.size() - 1;
    }

    //Log level 0
    if (boxing->has_overlaps(boxes)) throw std::logic_error("Check failed");

    //Log level 1
    if (loglevel >= 1)
    {
        std::cout << "Time      : " << std::setprecision(5) << timer << "s" << std::endl;
        std::cout << "Boxes     : " << boxes.size() << std::endl;
        std::cout << "Iterations: " << iteration_count << std::endl;
        std::cout << "Occupation: " << std::setprecision(5) <<
            100.0 * boxing->occupied_area(boxes) / (boxes.size() * boxing->box_area()) << "%" << std::endl;
    }

    //Log level 2
    if (loglevel >= 2) for (unsigned int i = 0; i < boxes.size(); i++)
    {
        std::cout << "Box " << i << ": " <<
            boxes[i].rectangles.size() << " rectangles, " <<
            std::setprecision(5) << 100.0 * boxing->occupied_area(boxes[i]) / boxing->box_area() << "% occupied" << std::endl;

        //Log level 3
        if (loglevel >= 3) for (unsigned int j = 0; j < boxes[i].rectangles.size(); j++)
        {
            const opt::Boxing::BoxedRectangle &r = boxes[i].rectangles[j];
            std::cout << "Rectangle " << j << ": ";
            if (r.transposed)
                std::cout << "[" << r.rectangle->height << ", " << r.rectangle->width << "] ";
            else
                std::cout << "[" << r.rectangle->width << ", " << r.rectangle->height << "] ";
            std::cout << "(" << r.x << ", " << r.y << ") " << std::endl;
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