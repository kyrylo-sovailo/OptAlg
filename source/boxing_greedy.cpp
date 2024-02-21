#include "../include/optalg/boxing_greedy.h"

opt::BoxingGreedy::BoxingGreedy(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, unsigned int seed,
    Metric metric)
    : Boxing(box_size, item_number, item_size_min, item_size_max, seed), _metric(metric)
{
}
    
const opt::BoxingGreedy::ElementContainer &opt::BoxingGreedy::elements() const
{
    return _rectangles;
}

bool opt::BoxingGreedy::can_join(const Solution &, const Element &) const
{
    return true;
}

opt::BoxingGreedy::Solution opt::BoxingGreedy::join(Solution &&solution, const Element &element) const
{
    _put_rectangle(element, &solution);
    return std::move(solution);
}

double opt::BoxingGreedy::weight(const Element &element) const
{
    if (_metric == Metric::max_size) return std::max(element.width, element.height);
    else if (_metric == Metric::min_size) return std::min(element.width, element.height);
    else return element.width * element.height;
}

std::vector<opt::Boxing::Box> opt::BoxingGreedy::get_boxes(const Solution &solution) const
{
    std::vector<opt::Boxing::Box> boxes;
    for (auto box = solution.begin(); box != solution.end(); box++) boxes.push_back(box->first);
    return boxes;
}