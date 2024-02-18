#include "../include/optalg/boxing_greedy.h"
#include <stdexcept>

opt::BoxingGreedy::BoxingGreedy(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, Metric metric)
    : Boxing(box_size, item_number, item_size_min, item_size_max), _metric(metric)
{
}
    
const opt::Boxing::ElementContainer &opt::BoxingGreedy::elements() const
{
    return _rectangles;
}

bool opt::BoxingGreedy::can_join(const Solution &solution, const Element &element) const
{
    return true;
}

opt::Boxing::Solution opt::BoxingGreedy::join(Solution &&solution, const Element &element) const
{
    //Try to fit in existing boxes
    for (auto box = solution.begin(); box != solution.end(); box++)
    {
        std::pair<bool, BoxedRectangle> fit = _fit(element, *box);
        if (fit.first)
        {
            box->rectangles.push_back(fit.second);
            return std::move(solution);
        }
    }

    //Fit in new box
    solution.push_back(Box());
    BoxedRectangle boxed(element);
    boxed.x = 0;
    boxed.y = 0;
    boxed.transposed = element.height > element.width;
    solution.back().rectangles.push_back(boxed);
    return std::move(solution);
}

double opt::BoxingGreedy::weight(const Element &element) const
{
    if (_metric == Metric::max_size) return std::max(element.width, element.height);
    else if (_metric == Metric::min_size) return std::min(element.width, element.height);
    else return element.width * element.height;
}