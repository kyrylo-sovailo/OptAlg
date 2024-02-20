#include "../include/optalg/boxing_neighborhood.h"
#include <random>
#include <vector>

opt::BoxingNeighborhoodOrder::BoxingNeighborhoodOrder(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max)
    : Boxing(box_size, item_number, item_size_min, item_size_max)
{
}

opt::BoxingNeighborhoodOrder::Solution opt::BoxingNeighborhoodOrder::initial() const
{
    std::vector<const Rectangle*> order(_rectangles.size());
    for (unsigned int i = 0; i < order.size(); i++) order[i] = &_rectangles[i];
    return order;
}

opt::BoxingNeighborhoodOrder::SolutionContainer opt::BoxingNeighborhoodOrder::neighbors(const Solution &solution)
{
    const unsigned int window = 2;
    std::vector<Solution> neighborhood;

    //Adding regular permutations
    for (unsigned int i = 0; i < solution.size(); i++)
    {
        for (unsigned int j = i + 1; j <= i + window && j < solution.size(); j++)
        {
            Solution permutation = solution;
            permutation[j] = solution[i];
            permutation[i] = solution[j];
            neighborhood.push_back(permutation);
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
        const double percentage = static_cast<double>(_occupied_space(boxes[box_i].first)) / (_box_size * _box_size - 1);
        boxes_empty[box_i] = percentage <= empty_threshold;
    }

    std::uniform_int_distribution<unsigned int> distribution(0, solution.size());
    for (unsigned int rectangle_i = 0; rectangle_i < solution.size(); rectangle_i++)
    {
        const unsigned int box_i = rectangle_affinity[rectangle_i];
        const bool empty = boxes_empty[box_i];
        if (empty)
        {
            Solution permutation = solution;
            const Rectangle *rectangle = solution[rectangle_i];
            const unsigned int new_rectangle_i = distribution(_engine);
            permutation.erase(permutation.begin() + rectangle_i);
            permutation.insert(permutation.begin() + new_rectangle_i, rectangle);
            neighborhood.push_back(permutation);
        }
    }

    return neighborhood;
}

double opt::BoxingNeighborhoodOrder::heuristic(const Solution &solution, unsigned int) const
{
    std::vector<Box> boxes = get_boxes(solution);
    if (boxes.empty()) return 0.0;
    else return solution.size() - static_cast<double>(_least_occupied_space(boxes)) / (_box_size * _box_size);
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