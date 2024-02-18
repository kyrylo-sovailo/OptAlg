#pragma once
#include <vector>
#include <set>

namespace opt
{
    ///Boxing problem
    class Boxing
    {
    public:
        //Specific
        struct Rectangle
        {
            unsigned int width, height;
            Rectangle(unsigned int width, unsigned int height);
        };
        
        struct BoxedRectangle
        {
            const Rectangle &rectangle;
            unsigned int x, y;
            bool transposed;
            BoxedRectangle(const Rectangle &rectangle, unsigned int x, unsigned int y, bool transposed);
        };
        
        struct Box
        {
            std::vector<BoxedRectangle> rectangles;
        };

    protected:
        unsigned int _box_size;
        std::vector<Rectangle> _rectangles;
        
        unsigned int _get_end_x(const BoxedRectangle &rectangle) const;
        unsigned int _get_end_y(const BoxedRectangle &rectangle) const;
        bool _fits(const BoxedRectangle &rectangle, const Box &box) const;
        bool _fits(const BoxedRectangle &rectangle, const std::vector<bool> &image) const;
        std::vector<bool> _create_image(const Box &box) const;
        std::pair<bool, BoxedRectangle> _fit(const Rectangle &rectangle, const Box &box) const;

    public:
        
        Boxing(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max);
        
        //Abstract
        typedef Rectangle Element;
        typedef std::vector<Element> ElementContainer;
        typedef std::vector<Box> Solution;
        typedef std::vector<Solution> SolutionContainer;
    };
}