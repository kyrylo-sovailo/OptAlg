#pragma once
#include "boxing.h"

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
    
        const ElementContainer &elements() const;
        bool can_join(const Solution &solution, const Element &element) const;
        Solution join(Solution &&solution, const Element &element) const;
        double weight(const Element &element) const;
    };
}