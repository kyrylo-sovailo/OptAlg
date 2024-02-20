#pragma once
#include "boxing.h"
#include <vector>
#include <utility>

namespace opt
{
    ///Boxing problem on which a greedy algorithm can be applied
    class BoxingGreedy : public Boxing
    {
    public:
        enum class Metric
        {
            max_size,
            min_size,
            area
        };

    protected:
        Metric _metric;

    public:
        BoxingGreedy(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, Metric metric);

        //Implementing greedy requirements
        typedef Rectangle Element;
        typedef std::vector<Element> ElementContainer;
        typedef std::vector<std::pair<Box, BoxImage>> Solution;
        typedef std::vector<Solution> SolutionContainer;
        const ElementContainer &elements() const;
        bool can_join(const Solution &solution, const Element &element) const;
        Solution join(Solution &&solution, const Element &element) const;
        double weight(const Element &element) const;

        //Getting specific data
        std::vector<Box> get_boxes(const Solution &solution) const;
    };
}