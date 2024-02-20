#include "../include/optalg/boxing_neighborhood.h"

unsigned int opt::BoxingNeighborhoodGeometryOverlap::_overlapping_area(const BoxedRectangle &a, const BoxedRectangle &b) const
{
    const unsigned int begin_x = std::max(a.x, b.x);
    const unsigned int begin_y = std::max(a.y, b.y);
    const unsigned int end_x = std::min(a.x_end(), b.x_end());
    const unsigned int end_y = std::min(a.y_end(), b.y_end());
    if (begin_x >= end_x || begin_y >= end_y) return 0;
    else return (end_x - begin_x) * (end_y - begin_y);
}

opt::BoxingNeighborhoodGeometryOverlap::BoxingNeighborhoodGeometryOverlap(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max,
    unsigned int window, unsigned int desired_iter)
    : Boxing(box_size, item_number, item_size_min, item_size_max), _window(window), _desired_iter(desired_iter)
{}

opt::BoxingNeighborhoodGeometryOverlap::Solution opt::BoxingNeighborhoodGeometryOverlap::initial()
{
    std::vector<Box> boxes(1);

    for (auto rectangle = _rectangles.cbegin(); rectangle != _rectangles.cend(); rectangle++)
    {
        //Randomly generate position
        std::uniform_int_distribution<unsigned int> x_distribution(0, _box_size - rectangle->width + 1);
        std::uniform_int_distribution<unsigned int> y_distribution(0, _box_size - rectangle->height + 1);
        BoxedRectangle boxed_rectangle(*rectangle, x_distribution(_engine), y_distribution(_engine), false);

        //Put in box
        boxes[0].rectangles.push_back(boxed_rectangle);
    }
    return boxes;
}

opt::BoxingNeighborhoodGeometryOverlap::SolutionContainer opt::BoxingNeighborhoodGeometryOverlap::neighbors(const Solution &solution) const
{
    std::vector<std::vector<Box>> neighborhood;

    //For every box
    for (unsigned int box_i = 0; box_i < solution.size(); box_i++)
    {
        //For every rectangle
        const Box &box = solution[box_i];
        for (unsigned int rectangle_i = 0; rectangle_i < box.rectangles.size(); rectangle_i++)
        {
            const BoxedRectangle &rectangle = box.rectangles[rectangle_i];

            //For every neighboring box
            for (unsigned int box_j = std::max(box_i, _window) - _window;
                box_j <= box_i + _window && box_j < solution.size();
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

                        //Check no-transpose move
                        if (_can_put_rectangle(move))
                        {
                            neighborhood.push_back(solution);
                            if (box_j != box_i)
                            {
                                neighborhood.back()[box_i].rectangles.erase(neighborhood.back()[box_i].rectangles.begin() + rectangle_i);
                                neighborhood.back()[box_j].rectangles.push_back(move);
                            }
                            else neighborhood.back()[box_i].rectangles[rectangle_i] = move;
                        }

                        //Check transpose move
                        BoxedRectangle transpose_move(*move.rectangle, move.x, move.y, !move.transposed);
                        if (_can_put_rectangle(transpose_move))
                        {
                            neighborhood.push_back(solution);
                            if (box_j != box_i)
                            {
                                neighborhood.back()[box_i].rectangles.erase(neighborhood.back()[box_i].rectangles.begin() + rectangle_i);
                                neighborhood.back()[box_j].rectangles.push_back(transpose_move);
                            }
                            else neighborhood.back()[box_i].rectangles[rectangle_i] = transpose_move;
                        }
                    }
                }
            }
        }
    }

    return neighborhood;
}

double opt::BoxingNeighborhoodGeometryOverlap::heuristic(const Solution &solution, unsigned int iter) const
{
    const double allowed_percentage = (iter < _desired_iter) ? (static_cast<double>(_desired_iter - iter) / _desired_iter) : 0.0;
    const double area_penalty = 1.0;

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
                const unsigned int overlap_area = _overlapping_area(*rectangle, *next_rectangle);
                const double percentage = static_cast<double>(overlap_area) / std::max(rectangle_area, next_area);
                if (percentage > allowed_percentage) penalty += area_penalty * (percentage - allowed_percentage);
            }
        }
    }

    if (solution.empty()) return 0.0;
    else return solution.size() - 1 + static_cast<double>(least_occupied_space(solution)) / (_box_size * _box_size) + penalty;
}

bool opt::BoxingNeighborhoodGeometryOverlap::good(const Solution &solution) const
{
    BoxImage image;
    for (auto box = solution.cbegin(); box != solution.cend(); box++)
    {
        if (image.empty()) image = _image_create(); else _image_clear(&image);
        for (auto rectangle = box->rectangles.cbegin(); rectangle != box->rectangles.cend(); rectangle++)
        {
            if (!_can_put_rectangle(*rectangle, image)) return false;
            _image_add(&image, *rectangle);
        }
    }
    return true;
}

std::vector<opt::Boxing::Box> opt::BoxingNeighborhoodGeometryOverlap::get_boxes(const Solution &solution) const
{
    return solution;
}