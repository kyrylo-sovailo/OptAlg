#include "../include/optalg/boxing_neighborhood.h"
#include <random>
#include <vector>

opt::BoxingNeighborhoodGeometry::BoxingNeighborhoodGeometry(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max)
    : Boxing(box_size, item_number, item_size_min, item_size_max)
{}

opt::BoxingNeighborhoodGeometry::Solution opt::BoxingNeighborhoodGeometry::initial()
{
    std::vector<Box> boxes;
    BoxImage image = _image_create();

    for (auto rectangle = _rectangles.cbegin(); rectangle != _rectangles.cend(); rectangle++)
    {
        //Randomly generate position
        std::uniform_int_distribution<unsigned int> x_distribution(0, _box_size - rectangle->width + 1);
        std::uniform_int_distribution<unsigned int> y_distribution(0, _box_size - rectangle->height + 1);
        BoxedRectangle boxed_rectangle(*rectangle, x_distribution(_engine), y_distribution(_engine), false);

        //Try to fit in last box
        const bool fit = !boxes.empty() && _can_put_rectangle(boxed_rectangle, image);

        //Create new box
        if (!fit)
        {
            boxes.push_back(Box());
            _image_clear(&image);
        }

        //Put in box
        boxes.back().rectangles.push_back(boxed_rectangle);
        _image_add(&image, boxed_rectangle);
    }
    return boxes;
}

opt::BoxingNeighborhoodGeometry::SolutionContainer opt::BoxingNeighborhoodGeometry::neighbors(const Solution &solution) const
{
    const unsigned int window = 2;
    BoxImage images[2 * window + 1];
    std::vector<std::vector<Box>> neighborhood;

    //Create cache 2 * window + 1; box_i++) 
    for (unsigned int box_j = 0; box_j <= window && box_j < solution.size(); box_j++)
    {
        images[window + box_j - 0] = _image_create();
        _image_add_all(&images[window + box_j - 0], solution[box_j]);
    }

    //For every box
    for (unsigned int box_i = 0; box_i < solution.size(); box_i++)
    {
        //For every rectangle
        const Box &box = solution[box_i];
        for (unsigned int rectangle_i = 0; rectangle_i < box.rectangles.size(); rectangle_i++)
        {
            const BoxedRectangle &rectangle = box.rectangles[rectangle_i];
            _image_remove(&images[window], rectangle);

            //For every neighboring box
            for (unsigned int box_j = std::max(box_i, window) - window;
                box_j <= box_i + window && box_j < solution.size();
                box_j++)
            {
                //For every neighboring y
                BoxedRectangle move = rectangle;
                for (move.y = std::max(rectangle.y, window) - window;
                    move.y <= rectangle.y + window;
                    move.y++)
                {
                    //For every neighboring x
                    for (move.x = std::max(rectangle.x, window) - window;
                        move.x <= rectangle.x + window;
                        move.x++)
                    {
                        //Dismiss no-move
                        if (box_j == box_i && move.y == rectangle.y && move.x == rectangle.x) continue;

                        //Check no-transpose move
                        if (_can_put_rectangle(move, images[window + box_j - box_i]))
                        {
                            neighborhood.push_back(solution);
                            neighborhood.back()[box_i].rectangles[rectangle_i] = move;
                        }

                        //Check transpose move
                        BoxedRectangle transpose_move(*move.rectangle, move.x, move.y, !move.transposed);
                        if (_can_put_rectangle(transpose_move, images[window + box_j - box_i]))
                        {
                            //TODO: shift rectangle
                            neighborhood.push_back(solution);
                            neighborhood.back()[box_i].rectangles[rectangle_i] = transpose_move;
                        }
                    }
                }
            }

            _image_add(&images[window], rectangle);
        }

        //Shift cache
        for (unsigned int i = 0; i < 2 * window; i++)
        {
            images[i] = std::move(images[i + 1]);
        }
        if ((box_i + 1) + window < solution.size())
        {
            images[2 * window] = _image_create();
            _image_add_all(&images[2 * window], solution[(box_i + 1) + window]);
        }
    }

    //Remove empty boxes
    for (auto neighbor = neighborhood.begin(); neighbor != neighborhood.end(); neighbor++)
    {
        for (unsigned int box_i = neighbor->size(); box_i > 0; box_i--)
        {
            if ((*neighbor)[box_i - 1].rectangles.empty()) neighbor->erase(neighbor->begin() + box_i - 1);
        }
    }

    return neighborhood;
}

double opt::BoxingNeighborhoodGeometry::heuristic(const Solution &solution, unsigned int) const
{
    if (solution.empty()) return 0.0;
    else return solution.size() - static_cast<double>(_least_occupied_space(solution)) / (_box_size * _box_size);
}

bool opt::BoxingNeighborhoodGeometry::good(const Solution &) const
{
    return true;
}

std::vector<opt::Boxing::Box> opt::BoxingNeighborhoodGeometry::get_boxes(const Solution &solution) const
{
    return solution;
}