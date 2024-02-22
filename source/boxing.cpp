#include "../include/optalg/boxing.h"
#include <random>

opt::Boxing::Rectangle::Rectangle(unsigned int width, unsigned int height)
    : width(width), height(height) {}

opt::Boxing::BoxedRectangle::BoxedRectangle(const Rectangle &rectangle, unsigned int x, unsigned int y, bool transposed)
    : rectangle(&rectangle), x(x), y(y), transposed(transposed) {}

unsigned int opt::Boxing::BoxedRectangle::x_end() const
{
    return x + (transposed ? rectangle->height : rectangle->width);
}

unsigned int opt::Boxing::BoxedRectangle::y_end() const
{
    return y + (transposed ? rectangle->width : rectangle->height);
}

opt::Boxing::BoxImage opt::Boxing::_image_create() const
{
    return BoxImage(_box_size * _box_size, false);
}

void opt::Boxing::_image_add(BoxImage *image, const BoxedRectangle &rectangle) const
{
    for (unsigned int y = rectangle.y; y < rectangle.y_end(); y++)
    {
        for (unsigned int x = rectangle.x; x < rectangle.x_end(); x++)
        {
            (*image)[_box_size * y + x] = true;
        }
    }
}

void opt::Boxing::_image_add_all(BoxImage *image, const Box &box) const
{
    for (auto rectangle = box.rectangles.cbegin(); rectangle != box.rectangles.cend(); rectangle++)
        _image_add(image, *rectangle);
}

void opt::Boxing::_image_remove(BoxImage *image, const BoxedRectangle &rectangle) const
{
    for (unsigned int y = rectangle.y; y < rectangle.y_end(); y++)
    {
        for (unsigned int x = rectangle.x; x < rectangle.x_end(); x++)
        {
            (*image)[_box_size * y + x] = false;
        }
    }
}

void opt::Boxing::_image_clear(BoxImage *image) const
{
    image->assign(_box_size * _box_size, false);
}

std::pair<bool, opt::Boxing::BoxedRectangle> opt::Boxing::_can_transpose_center(const BoxedRectangle &rectangle) const
{
    const unsigned int width = rectangle.transposed ? rectangle.rectangle->height : rectangle.rectangle->width;
    const unsigned int height = rectangle.transposed ? rectangle.rectangle->width : rectangle.rectangle->height;
    if (rectangle.x + width / 2 >= height / 2 && rectangle.y + height / 2 >= width / 2)
    {
        BoxedRectangle transposed(*rectangle.rectangle,
        rectangle.x + width / 2 - height / 2,
        rectangle.y + height / 2 - width / 2,
        !rectangle.transposed);
        return { true, transposed};
    }
    else return { false, rectangle };
}

bool opt::Boxing::_can_put_rectangle(const BoxedRectangle &rectangle) const
{
    return rectangle.x_end() <= _box_size && rectangle.y_end() <= _box_size;
}

bool opt::Boxing::_can_put_rectangle(const BoxedRectangle &rectangle, const BoxImage &image) const
{
    if (!_can_put_rectangle(rectangle)) return false;

    for (unsigned int y = rectangle.y; y < rectangle.y_end(); y++)
    {
        for (unsigned int x = rectangle.x; x < rectangle.x_end(); x++)
        {
            if (image[_box_size * y + x]) return false;
        }
    }
    return true;
}

std::pair<bool, opt::Boxing::BoxedRectangle> opt::Boxing::_can_put_rectangle(const Rectangle &rectangle, const BoxImage &image) const
{
    //Try to fit horizontally
    const bool tall = rectangle.height > rectangle.width;
    const unsigned int width = tall ? rectangle.height : rectangle.width;
    const unsigned int height = tall ? rectangle.width : rectangle.height;
    BoxedRectangle boxed_rectangle(rectangle, 0, 0, tall);
    for (boxed_rectangle.y = 0; boxed_rectangle.y < _box_size - height + 1; boxed_rectangle.y++)
    {
        for (boxed_rectangle.x = 0; boxed_rectangle.x < _box_size - width + 1; boxed_rectangle.x++)
        {
            if (_can_put_rectangle(boxed_rectangle, image)) return { true, boxed_rectangle };
        }
    }

    //Try to fit vertically
    boxed_rectangle.transposed = !tall;
    for (boxed_rectangle.y = 0; boxed_rectangle.y < _box_size - width + 1; boxed_rectangle.y++)
    {
        for (boxed_rectangle.x = 0; boxed_rectangle.x < _box_size - height + 1; boxed_rectangle.x++)
        {
            if (_can_put_rectangle(boxed_rectangle, image)) return { true, boxed_rectangle };
        }
    }

    return { false, boxed_rectangle };
}

unsigned int opt::Boxing::box_size() const
{
    return _box_size;
}

unsigned int opt::Boxing::box_area() const
{
    return _box_size * _box_size;
}

unsigned int opt::Boxing::_put_rectangle(const Rectangle &rectangle, std::vector<std::pair<Box, BoxImage>> *boxes) const
{
    //Try to fit in existing boxes
    for (unsigned int box_i = 0; box_i < boxes->size(); box_i++)
    {
        std::pair<Box, BoxImage> &box = (*boxes)[box_i];
        std::pair<bool, BoxedRectangle> fit = _can_put_rectangle(rectangle, box.second);
        if (fit.first)
        {
            box.first.rectangles.push_back(fit.second);
            _image_add(&box.second, fit.second);
            return box_i;
        }
    }

    //Fit in new box
    boxes->push_back({ Box(), _image_create() });
    BoxedRectangle boxed_rectangle(rectangle, 0, 0, rectangle.height > rectangle.width);
    boxes->back().first.rectangles.push_back(boxed_rectangle);
    _image_add(&boxes->back().second, boxed_rectangle);
    return boxes->size() - 1;
}

opt::Boxing::Boxing(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, unsigned int seed)
    : _box_size(box_size)
{
    std::uniform_int_distribution<unsigned int> distribution(item_size_min, item_size_max);
    std::default_random_engine engine(seed);

    for (unsigned int i = 0; i < item_number; i++)
    {
        _rectangles.push_back(Rectangle(distribution(engine), distribution(engine)));
    }
}

double opt::Boxing::energy(const std::vector<Box> &boxes, unsigned int cycle) const
{
    double energy = 0;
    for (unsigned int box_i = 0; box_i < boxes.size(); box_i++)
    {
        const Box &box = boxes[box_i];
        for (auto rectangle = box.rectangles.cbegin(); rectangle != box.rectangles.cend(); rectangle++)
        {
            double height = box_i * _box_size;
            if (box_i % cycle == 0) height += static_cast<double>(rectangle->y + rectangle->y_end()) / 2;
            else if (box_i % cycle == 1) height += static_cast<double>(rectangle->x + rectangle->x_end()) / 2;
            else if (box_i % cycle == 2) height += static_cast<double>(_box_size - rectangle->y - rectangle->y_end()) / 2;
            else height += static_cast<double>(_box_size - rectangle->x - rectangle->x_end()) / 2;
            energy += height * rectangle->rectangle->width * rectangle->rectangle->height;
        }
    }
    return energy;
}

bool opt::Boxing::has_overlaps(const std::vector<Box> &boxes) const
{
    BoxImage image;
    for (auto box = boxes.cbegin(); box != boxes.cend(); box++)
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

unsigned int opt::Boxing::overlap_area(const BoxedRectangle &a, const BoxedRectangle &b) const
{
    const unsigned int begin_x = std::max(a.x, b.x);
    const unsigned int begin_y = std::max(a.y, b.y);
    const unsigned int end_x = std::min(a.x_end(), b.x_end());
    const unsigned int end_y = std::min(a.y_end(), b.y_end());
    if (begin_x < end_x && begin_y < end_y) return (end_x - begin_x) * (end_y - begin_y);
    else return 0;
}

unsigned int opt::Boxing::overlap_area(const std::vector<Box> &boxes) const
{
    unsigned int overlaps = 0;

    //For every box
    for (auto box = boxes.cbegin(); box != boxes.cend(); box++)
    {
        //For every rectangle
        for (auto rectangle = box->rectangles.cbegin(); rectangle != box->rectangles.cend(); rectangle++)
        {
            //For every next rectangle
            for (auto next_rectangle = rectangle + 1; next_rectangle != box->rectangles.cend(); next_rectangle++)
            {
                overlaps += overlap_area(*rectangle, *next_rectangle);
            }
        }
    }
    return overlaps;
}

unsigned int opt::Boxing::occupied_area(const Box &box) const
{
    unsigned int occupied = 0;
    for (auto rectangle = box.rectangles.cbegin(); rectangle != box.rectangles.cend(); rectangle++)
    {
        occupied += (rectangle->rectangle->width * rectangle->rectangle->height);
    }
    return occupied;
}

unsigned int opt::Boxing::occupied_area(const std::vector<Box> &boxes, double max_occupation) const
{
    unsigned int occupied = 0;
    for (auto box = boxes.cbegin(); box != boxes.cend(); box++)
    {
        const double occupation = static_cast<double>(occupied_area(*box)) / box_area();
        if (occupation <= max_occupation) occupied += occupied_area(*box);
    }
    return occupied;
}

unsigned int opt::Boxing::least_occupied_area(const std::vector<Box> &boxes) const
{
    if (boxes.empty()) return 0;
    unsigned int least_occupied = occupied_area(boxes.front());
    for (auto box = boxes.cbegin() + 1; box != boxes.cend(); box++)
    {
        least_occupied = std::min(least_occupied, occupied_area(*box));
    }
    return least_occupied;
}

unsigned int opt::Boxing::rectangle_number(const std::vector<Box> &boxes, double max_occupation) const
{
    unsigned int number = 0;
    for (auto box = boxes.cbegin(); box != boxes.cend(); box++)
    {
        const double occupation = static_cast<double>(occupied_area(*box)) / box_area();
        if (occupation <= max_occupation) number += box->rectangles.size();
    }
    return number;
}

unsigned int opt::Boxing::least_rectangle_number(const std::vector<Box> &boxes, double max_occupation) const
{
    if (boxes.empty()) return 0;
    unsigned int number = boxes.front().rectangles.size();
    for (auto box = boxes.cbegin() + 1; box != boxes.cend(); box++)
    {
        const double occupation = static_cast<double>(occupied_area(*box)) / box_area();
        if (occupation <= max_occupation) number = std::min(number, static_cast<unsigned int>(box->rectangles.size()));
    }
    return number;
}