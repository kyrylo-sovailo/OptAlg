#include "../include/optalg/boxing.h"
#include <random>
#include <algorithm>
#include <type_traits>

opt::Boxing::Rectangle::Rectangle(unsigned int width, unsigned int height) : width(width), height(height) {}

opt::Boxing::BoxedRectangle::BoxedRectangle(const Rectangle &rectangle) : rectangle(rectangle) {}

unsigned int opt::Boxing::_get_end_x(const BoxedRectangle &rectangle) const
{
    return rectangle.x + (rectangle.transposed ? rectangle.rectangle.height : rectangle.rectangle.width);
}

unsigned int opt::Boxing::_get_end_y(const BoxedRectangle &rectangle) const
{
    return rectangle.y + (rectangle.transposed ? rectangle.rectangle.width : rectangle.rectangle.height);
}

bool opt::Boxing::_fits(const BoxedRectangle &rectangle, const Box &box) const
{
    //Check borders
    const unsigned int end_x = _get_end_x(rectangle);
    const unsigned int end_y = _get_end_y(rectangle);
    if (end_x > _box_size || end_y > _box_size) return false;
    
    //Check existing rectangles
    for (auto rect = box.rectangles.cbegin(); rect != box.rectangles.cend(); rect++)
    {
        if (!(end_x <= rect->x || rectangle.x >= _get_end_x(*rect))) return false;
        if (!(end_y <= rect->y || rectangle.y >= _get_end_y(*rect))) return false;
    }
    return  true;
}

bool opt::Boxing::_fits(const BoxedRectangle &rectangle, const std::vector<bool> &image) const
{
    const unsigned int end_x = _get_end_x(rectangle);
    const unsigned int end_y = _get_end_y(rectangle);
    for (unsigned int xi = rectangle.x; xi < end_x; xi++)
    {
        for (unsigned int yi = rectangle.y; yi < end_y; yi++)
        {
            if (image[_box_size * xi + yi]) return false;
        }
    }
    return true;
}

std::vector<bool> opt::Boxing::_create_image(const Box &box) const
{
    std::vector<bool> image(_box_size * _box_size, false);
    for (auto rect = box.rectangles.cbegin(); rect != box.rectangles.cend(); rect++)
    {
        const unsigned int rect_end_x = rect->x + (rect->transposed ? rect->rectangle.height : rect->rectangle.width);
        const unsigned int rect_end_y = rect->y + (rect->transposed ? rect->rectangle.width : rect->rectangle.height);
        for (unsigned int xi = rect->x; xi < rect_end_x; xi++)
        {
            for (unsigned int yi = rect->y; yi < rect_end_y; yi++)
            {
                image[_box_size * xi + yi] = true;
            }
        }
    }
    return std::move(image);
}

std::pair<bool, opt::Boxing::BoxedRectangle> opt::Boxing::_fit(const Rectangle &rectangle, const Box &box) const
{
    std::vector<bool> image = _create_image(box);

    const bool tall = rectangle.height > rectangle.width;
    BoxedRectangle boxed_rectangle(rectangle);
    for (unsigned int xi = 0; xi < _box_size - rectangle.width; xi++)
    {
        boxed_rectangle.x = xi;
        for (unsigned int yi = 0; yi < _box_size - rectangle.height; yi++)
        {
            boxed_rectangle.y = yi;
            boxed_rectangle.transposed = tall;
            if (_fits(rectangle, image)) return { true, boxed_rectangle }; //Check wide
            boxed_rectangle.transposed = !tall;
            if (_fits(rectangle, image)) return { true, boxed_rectangle }; //Check tall
        }
    }
    return { false, boxed_rectangle };
}

opt::Boxing::Boxing(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max)
    : _box_size(box_size)
{
    std::default_random_engine engine;
    std::uniform_int_distribution<unsigned int> distribution(item_size_min, item_size_max);
    for (unsigned int i = 0; i < item_number; i++)
    {
        _rectangles.push_back(Rectangle(distribution(engine), distribution(engine)));
    }
}