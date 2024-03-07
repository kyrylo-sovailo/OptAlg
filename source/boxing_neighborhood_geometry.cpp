#include "../include/optalg/boxing_neighborhood.h"
#include <random>
#include <vector>

opt::BoxingNeighborhoodGeometry::BoxingNeighborhoodGeometry(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max,
    unsigned int seed, unsigned int window, unsigned int hwindow)
    : Boxing(box_size, item_number, item_size_min, item_size_max, seed), _window(window), _hwindow(hwindow)
{}

opt::BoxingNeighborhoodGeometry::Solution opt::BoxingNeighborhoodGeometry::initial(unsigned int seed) const
{
    std::vector<Box> boxes;
    BoxImage image = _image_create();
    std::default_random_engine engine(seed);

    for (auto rectangle = _rectangles.cbegin(); rectangle != _rectangles.cend(); rectangle++)
    {
        //Randomly generate position
        std::uniform_int_distribution<unsigned int> x_distribution(0, _box_size - rectangle->width);
        std::uniform_int_distribution<unsigned int> y_distribution(0, _box_size - rectangle->height);
        BoxedRectangle boxed_rectangle(*rectangle, x_distribution(engine), y_distribution(engine), false);

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

opt::BoxingNeighborhoodGeometry::SolutionContainer opt::BoxingNeighborhoodGeometry::neighbors(const Solution &solution,
    std::default_random_engine &, unsigned int id, unsigned int nthreads) const
{
    std::vector<BoxImage> images((_hwindow != 0) ? (2 * _hwindow + 1) : (solution.size()));
    std::vector<std::vector<Box>> neighborhood;

    //Create cache
    const unsigned int begin_box_i = solution.size() * id / nthreads;
    const unsigned int end_box_i = solution.size() * (id + 1) / nthreads;
    for (unsigned int box_j = ((_hwindow != 0) ? (std::max(begin_box_i, _hwindow) - _hwindow) : 0);
        ((_hwindow != 0) ? (box_j <= begin_box_i + _hwindow) : true) && box_j < solution.size();
        box_j++)
    {
        BoxImage &dest_image = (_hwindow != 0) ? images[_hwindow + box_j - begin_box_i] : images[box_j];
        dest_image = _image_create();
        _image_add_all(&dest_image, solution[box_j]);
    }

    //For every box
    for (unsigned int box_i = begin_box_i; box_i < end_box_i; box_i++)
    {
        const Box &box = solution[box_i];
        BoxImage &image = (_hwindow != 0) ? images[_hwindow] : images[box_i];

        //For every rectangle
        for (unsigned int rectangle_i = 0; rectangle_i < box.rectangles.size(); rectangle_i++)
        {
            const BoxedRectangle &rectangle = box.rectangles[rectangle_i];
            _image_remove(&image, rectangle);

            //For every neighboring box
            for (unsigned int box_j = ((_hwindow != 0) ? (std::max(box_i, _hwindow) - _hwindow) : 0);
                ((_hwindow != 0) ? (box_j <= box_i + _hwindow) : true) && box_j < solution.size();
                box_j++)
            {
                const BoxImage &dest_image = (_hwindow != 0) ? images[_hwindow + box_j - box_i] : images[box_j];

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
                        if (_can_put_rectangle(move, dest_image))
                        {
                            neighborhood.push_back(solution);
                            if (box_j != box_i)
                            {
                                neighborhood.back()[box_i].rectangles.erase(neighborhood.back()[box_i].rectangles.begin() + rectangle_i);
                                neighborhood.back()[box_j].rectangles.push_back(move);
                            }
                            else neighborhood.back()[box_i].rectangles[rectangle_i] = move;
                        }

                        //Check transposed move
                        std::pair<bool, BoxedRectangle> transposed_move = _can_transpose_center(move);
                        if (transposed_move.first && _can_put_rectangle(transposed_move.second, dest_image))
                        {
                            neighborhood.push_back(solution);
                            if (box_j != box_i)
                            {
                                neighborhood.back()[box_i].rectangles.erase(neighborhood.back()[box_i].rectangles.begin() + rectangle_i);
                                neighborhood.back()[box_j].rectangles.push_back(transposed_move.second);
                            }
                            else neighborhood.back()[box_i].rectangles[rectangle_i] = transposed_move.second;
                        }
                    }
                }
            }

            _image_add(&image, rectangle);
        }

        //Shift cache
        if (_hwindow != 0)
        {
            for (unsigned int i = 0; i < images.size() - 1; i++)
            {
                images[i] = std::move(images[i + 1]);
            }
            if ((box_i + 1) + _hwindow < solution.size())
            {
                images.back() = _image_create();
                _image_add_all(&images.back(), solution[(box_i + 1) + _hwindow]);
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

double opt::BoxingNeighborhoodGeometry::heuristic(const Solution &solution, unsigned int) const
{
    return energy(solution);
}

bool opt::BoxingNeighborhoodGeometry::good(const Solution &, unsigned int) const
{
    return true;
}

std::vector<opt::Boxing::Box> opt::BoxingNeighborhoodGeometry::get_boxes(const Solution &solution) const
{
    return solution;
}