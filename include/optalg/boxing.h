#pragma once
#include <vector>
#include <utility>
#include <random>

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
        std::default_random_engine _engine;
        
        //Image manipulation
        BoxImage _image_create() const;
        void _image_add(BoxImage *image, const BoxedRectangle &rectangle) const;
        void _image_add_all(BoxImage *image, const Box &box) const;
        void _image_remove(BoxImage *image, const BoxedRectangle &rectangle) const;
        void _image_clear(BoxImage *image) const;

        //Putting rectangles in boxes
        bool _can_put_rectangle(const BoxedRectangle &rectangle) const;
        bool _can_put_rectangle(const BoxedRectangle &rectangle, const BoxImage &image) const;
        std::pair<bool, BoxedRectangle> _can_put_rectangle(const Rectangle &rectangle, const BoxImage &image) const;
        unsigned int _put_rectangle(const Rectangle &rectangle, std::vector<std::pair<Box, BoxImage>> *boxes) const;

    public:
        Boxing(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max);
        unsigned int box_size() const;
        unsigned int occupied_space(const Box &box) const;
        unsigned int least_occupied_space(const std::vector<Box> &boxes) const;
    };
}