#include "../include/optalg/boxing_neighborhood.h"
#include <algorithm>
#include <random>
#include <vector>

opt::BoxingNeighborhoodOrder::BoxingNeighborhoodOrder(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max,
    unsigned int seed, unsigned int window)
    : Boxing(box_size, item_number, item_size_min, item_size_max, seed), _window(window)
{
}

opt::BoxingNeighborhoodOrder::Solution opt::BoxingNeighborhoodOrder::initial(unsigned int seed) const
{
    std::vector<const Rectangle*> order(_rectangles.size());
    for (unsigned int i = 0; i < order.size(); i++) order[i] = &_rectangles[i];
    std::default_random_engine engine(seed);
    std::shuffle(order.begin(), order.end(), engine);
    return order;
}

opt::BoxingNeighborhoodOrder::SolutionContainer opt::BoxingNeighborhoodOrder::neighbors(const Solution &solution,
    std::default_random_engine &engine, unsigned int id, unsigned int nthreads) const
{
    std::vector<Solution> neighborhood;

    //Adding regular permutations
    const unsigned int begin_rectangle_i = solution.size() * id / nthreads;
    const unsigned int end_rectangle_i = solution.size() * (id + 1) / nthreads;
    for (unsigned int rectangle_i = begin_rectangle_i; rectangle_i < end_rectangle_i; rectangle_i++)
    {
        for (unsigned int new_rectangle_i = rectangle_i + 1; new_rectangle_i <= rectangle_i + _window && new_rectangle_i < solution.size(); new_rectangle_i++)
        {
            neighborhood.push_back(solution);
            neighborhood.back()[new_rectangle_i] = solution[rectangle_i];
            neighborhood.back()[rectangle_i] = solution[new_rectangle_i];
        }
    }

    //Randomly inserting rectangles from empty boxes
    const double empty_threshold = 0.4;
    std::vector<std::pair<Box, BoxImage>> boxes;
    std::vector<unsigned int> rectangle_affinity(solution.size());
    for (unsigned int rectangle_i = 0; rectangle_i < solution.size(); rectangle_i++)
    {
        const Rectangle &rectangle = *solution[rectangle_i];
        rectangle_affinity[rectangle_i] = _put_rectangle(rectangle, &boxes);
    }

    std::vector<bool> boxes_empty(boxes.size());
    for (unsigned int box_i = 0; box_i < boxes.size(); box_i++)
    {
        const double percentage = static_cast<double>(occupied_area(boxes[box_i].first)) / (_box_size * _box_size - 1);
        boxes_empty[box_i] = percentage <= empty_threshold;
    }

    std::uniform_int_distribution<unsigned int> distribution(0, solution.size());
    for (unsigned int rectangle_i = begin_rectangle_i; rectangle_i < end_rectangle_i; rectangle_i++)
    {
        const unsigned int box_i = rectangle_affinity[rectangle_i];
        const bool empty = boxes_empty[box_i];
        if (empty)
        {
            const Rectangle *rectangle = solution[rectangle_i];
            const unsigned int new_rectangle_i = distribution(engine);
            neighborhood.push_back(solution);
            neighborhood.back().erase(neighborhood.back().begin() + rectangle_i);
            neighborhood.back().insert(neighborhood.back().begin() + new_rectangle_i, rectangle);
        }
    }

    return neighborhood;
}

double opt::BoxingNeighborhoodOrder::heuristic(const Solution &solution, unsigned int) const
{
    std::vector<Box> boxes = get_boxes(solution);
    return energy(boxes);
}

bool opt::BoxingNeighborhoodOrder::good(const Solution &) const
{
    return true;
}

std::vector<opt::Boxing::Box> opt::BoxingNeighborhoodOrder::get_boxes(const Solution &solution) const
{
    //Build
    std::vector<std::pair<Box, BoxImage>> boxes;
    for (auto rectangle = solution.cbegin(); rectangle != solution.cend(); rectangle++)
    {
        _put_rectangle(**rectangle, &boxes);
    }

    //Extract boxes
    std::vector<opt::Boxing::Box> boxes_only;
    for (auto box = boxes.begin(); box != boxes.end(); box++) boxes_only.push_back(box->first);
    return boxes_only;
}