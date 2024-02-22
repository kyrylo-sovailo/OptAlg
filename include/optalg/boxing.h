#pragma once
#include <vector>
#include <utility>

namespace opt
{
    ///Boxing problem
    class Boxing
    {
    public:
        struct Rectangle
        {
            unsigned int width, height;
            Rectangle(unsigned int width, unsigned int height);
        };
        
        struct BoxedRectangle
        {
            const Rectangle *rectangle;
            unsigned int x, y;
            bool transposed;
            BoxedRectangle(const Rectangle &rectangle, unsigned int x, unsigned int y, bool transposed);
            unsigned int x_end() const;
            unsigned int y_end() const;
        };
        
        struct Box
        {
            std::vector<BoxedRectangle> rectangles;
        };

        typedef std::vector<bool> BoxImage;

    protected:
        unsigned int _box_size;
        std::vector<Rectangle> _rectangles;
        
        //Image manipulation
        BoxImage _image_create() const;
        void _image_add(BoxImage *image, const BoxedRectangle &rectangle) const;
        void _image_add_all(BoxImage *image, const Box &box) const;
        void _image_remove(BoxImage *image, const BoxedRectangle &rectangle) const;
        void _image_clear(BoxImage *image) const;

        //Putting rectangles in boxes
        std::pair<bool, BoxedRectangle> _can_transpose_center(const BoxedRectangle &rectangle) const;
        bool _can_put_rectangle(const BoxedRectangle &rectangle) const;
        bool _can_put_rectangle(const BoxedRectangle &rectangle, const BoxImage &image) const;
        std::pair<bool, BoxedRectangle> _can_put_rectangle(const Rectangle &rectangle, const BoxImage &image) const;
        unsigned int _put_rectangle(const Rectangle &rectangle, std::vector<std::pair<Box, BoxImage>> *boxes) const;

    public:
        Boxing(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, unsigned int seed);
        unsigned int box_size() const;
        unsigned int box_area() const;

        //Heuristic helpers
        double energy(const std::vector<Box> &boxes, unsigned int cycle = 1) const;
        bool has_overlaps(const std::vector<Box> &boxes) const;
        unsigned int overlap_area(const BoxedRectangle &a, const BoxedRectangle &b) const;
        unsigned int overlap_area(const std::vector<Box> &boxes) const;
        unsigned int occupied_area(const Box &box) const;
        unsigned int occupied_area(const std::vector<Box> &boxes, double max_occupation = 1.0) const;
        unsigned int least_occupied_area(const std::vector<Box> &boxes) const;
        unsigned int rectangle_number(const std::vector<Box> &boxes, double max_occupation = 1.0) const;
        unsigned int least_rectangle_number(const std::vector<Box> &boxes, double max_occupation = 1.0) const;
    };
}