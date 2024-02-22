#include "../include/optalg/boxing_neighborhood.h"
#include <random>

opt::BoxingNeighborhoodGeometryOverlap::BoxingNeighborhoodGeometryOverlap(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max,
    unsigned int seed, unsigned int window, unsigned int hwindow, unsigned int desired_iter)
    : Boxing(box_size, item_number, item_size_min, item_size_max, seed), _window(window),  _hwindow(hwindow), _desired_iter(desired_iter)
{}

opt::BoxingNeighborhoodGeometryOverlap::Solution opt::BoxingNeighborhoodGeometryOverlap::initial(unsigned int seed) const
{
    std::vector<Box> boxes(1);
    std::default_random_engine engine(seed);

    for (auto rectangle = _rectangles.cbegin(); rectangle != _rectangles.cend(); rectangle++)
    {
        //Randomly generate position
        std::uniform_int_distribution<unsigned int> x_distribution(0, _box_size - rectangle->width + 1);
        std::uniform_int_distribution<unsigned int> y_distribution(0, _box_size - rectangle->height + 1);
        BoxedRectangle boxed_rectangle(*rectangle, x_distribution(engine), y_distribution(engine), false);

        //Put in box
        boxes[0].rectangles.push_back(boxed_rectangle);
    }
    return boxes;
}

opt::BoxingNeighborhoodGeometryOverlap::SolutionContainer opt::BoxingNeighborhoodGeometryOverlap::neighbors(const Solution &solution,
    std::default_random_engine &, unsigned int id, unsigned int nthreads) const
{
    std::vector<std::vector<Box>> neighborhood;

    //For every box
    const unsigned int begin_box_i = solution.size() * id / nthreads;
    const unsigned int end_box_i = solution.size() * (id + 1) / nthreads;
    for (unsigned int box_i = begin_box_i; box_i < end_box_i; box_i++)
    {
        //For every rectangle
        const Box &box = solution[box_i];
        for (unsigned int rectangle_i = 0; rectangle_i < box.rectangles.size(); rectangle_i++)
        {
            const BoxedRectangle &rectangle = box.rectangles[rectangle_i];

            //For every neighboring box
            for (unsigned int box_j = ((_hwindow != 0) ? (std::max(box_i, _hwindow) - _hwindow) : 0);
                ((_hwindow != 0) ? (box_j <= box_i + _hwindow) : true) && box_j < solution.size() + 1;
                box_j++)
            {
                //For every neighboring y
                BoxedRectangle move = rectangle;
                for (move.y = std::max(rectangle.y, _window) - _window;
                    move.y <= rectangle.y + _window;
                    move.y++)
                {
                    //For every neighboring x
                    for (move.x = std::max(rectangle.x, _window) - _window;
                        move.x <= rectangle.x + _window;
                        move.x++)
                    {
                        //Dismiss no-move
                        if (box_j == box_i && move.y == rectangle.y && move.x == rectangle.x) continue;

                        //Check non-transposed move
                        if (_can_put_rectangle(move))
                        {
                            neighborhood.push_back(solution);
                            if (box_j != box_i)
                            {
                                if (box_j == neighborhood.back().size()) neighborhood.back().push_back(Box());
                                neighborhood.back()[box_i].rectangles.erase(neighborhood.back()[box_i].rectangles.begin() + rectangle_i);
                                neighborhood.back()[box_j].rectangles.push_back(move);
                            }
                            else neighborhood.back()[box_i].rectangles[rectangle_i] = move;
                        }

                        //Check transposed move
                        std::pair<bool, BoxedRectangle> transposed_move = _can_transpose_center(move);
                        if (transposed_move.first && _can_put_rectangle(transposed_move.second))
                        {
                            neighborhood.push_back(solution);
                            if (box_j != box_i)
                            {
                                if (box_j == neighborhood.back().size()) neighborhood.back().push_back(Box());
                                neighborhood.back()[box_i].rectangles.erase(neighborhood.back()[box_i].rectangles.begin() + rectangle_i);
                                neighborhood.back()[box_j].rectangles.push_back(transposed_move.second);
                            }
                            else neighborhood.back()[box_i].rectangles[rectangle_i] = transposed_move.second;
                        }
                    }
                }
            }
        }
    }

    //Remove empty boxes
    for (auto neighbor = neighborhood.begin(); neighbor != neighborhood.end(); neighbor++)
    {
        for (unsigned int box_i = neighbor->size(); box_i > 0; box_i--)
        {
            if ((*neighbor)[box_i - 1].rectangles.empty())
            {
                neighbor->erase(neighbor->begin() + box_i - 1);
            }
        }
    }

    return neighborhood;
}

double opt::BoxingNeighborhoodGeometryOverlap::heuristic(const Solution &solution, unsigned int iter) const
{
    const double allowed_percentage = (iter < _desired_iter) ? (static_cast<double>(_desired_iter - iter) / _desired_iter) : 0.0;
    const double percentage_penalty = 1000 * 1000 * 1000;

    //For every box
    double penalty = 0.0;
    for (auto box = solution.cbegin(); box != solution.cend(); box++)
    {
        //For every rectangle
        for (auto rectangle = box->rectangles.cbegin(); rectangle != box->rectangles.cend(); rectangle++)
        {
            const unsigned int rectangle_area = rectangle->rectangle->width * rectangle->rectangle->height;

            //For every next rectangle
            for (auto next_rectangle = rectangle + 1; next_rectangle != box->rectangles.cend(); next_rectangle++)
            {
                const unsigned int next_area = next_rectangle->rectangle->width * next_rectangle->rectangle->height;
                const unsigned int overlaps = overlap_area(*rectangle, *next_rectangle);
                const double percentage = static_cast<double>(overlaps) / std::max(rectangle_area, next_area);
                if (percentage > allowed_percentage) penalty += percentage_penalty * (percentage - allowed_percentage);
            }
        }
    }

    return energy(solution) + penalty;
}

bool opt::BoxingNeighborhoodGeometryOverlap::good(const Solution &solution) const
{
    return !has_overlaps(solution);
}

std::vector<opt::Boxing::Box> opt::BoxingNeighborhoodGeometryOverlap::get_boxes(const Solution &solution) const
{
    return solution;
}