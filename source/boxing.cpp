#include "../include/optalg/boxing.h"
#include <random>
#include <algorithm>
#include <type_traits>

opt::Boxing::Rectangle::Rectangle(unsigned int width, unsigned int height)
    : width(width), height(height) {}

opt::Boxing::BoxedRectangle::BoxedRectangle(const Rectangle &rectangle, unsigned int x, unsigned int y, bool transposed)
    : rectangle(rectangle), x(x), y(y), transposed(transposed) {}

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
    //Check borders
    const unsigned int end_x = _get_end_x(rectangle);
    const unsigned int end_y = _get_end_y(rectangle);
    if (end_x > _box_size || end_y > _box_size) return false;

    //Check existing rectangles
    for (unsigned int y = rectangle.y; y < end_y; y++)
    {
        for (unsigned int x = rectangle.x; x < end_x; x++)
        {
            if (image[_box_size * y + x]) return false;
        }
    }
    return true;
}

std::vector<bool> opt::Boxing::_create_image(const Box &box) const
{
    std::vector<bool> image(_box_size * _box_size, false);
    for (auto rectangle = box.rectangles.cbegin(); rectangle != box.rectangles.cend(); rectangle++)
    {
        const unsigned int end_x = _get_end_x(*rectangle);
        const unsigned int end_y = _get_end_y(*rectangle);
        for (unsigned int yi = rectangle->y; yi < end_y; yi++)
        {
            for (unsigned int xi = rectangle->x; xi < end_x; xi++)
            {
                image[_box_size * yi + xi] = true;
            }
        }
    }
    return image;
}

std::pair<bool, opt::Boxing::BoxedRectangle> opt::Boxing::_fit(const Rectangle &rectangle, const Box &box) const
{
    //Create image
    std::vector<bool> image = _create_image(box);

    //Try to fit horizontally
    const bool tall = rectangle.height > rectangle.width;
    const unsigned int width = tall ? rectangle.height : rectangle.width;
    const unsigned int height = tall ? rectangle.width : rectangle.height;
    BoxedRectangle boxed_rectangle(rectangle, 0, 0, tall);
    for (boxed_rectangle.y = 0; boxed_rectangle.y < _box_size - height + 1; boxed_rectangle.y++)
    {
        for (boxed_rectangle.x = 0; boxed_rectangle.x < _box_size - width + 1; boxed_rectangle.x++)
        {
            if (_fits(boxed_rectangle, image)) return { true, boxed_rectangle };
        }
    }

    //Try to fit vertically
    boxed_rectangle.transposed = !tall;
    for (boxed_rectangle.y = 0; boxed_rectangle.y < _box_size - width + 1; boxed_rectangle.y++)
    {
        for (boxed_rectangle.x = 0; boxed_rectangle.x < _box_size - height + 1; boxed_rectangle.x++)
        {
            if (_fits(boxed_rectangle, image)) return { true, boxed_rectangle };
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