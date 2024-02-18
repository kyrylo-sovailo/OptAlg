#pragma once
#include "boxing.h"

namespace opt
{
    ///Boxing problem on which a local search algorithm can be applied
    class BoxingNeighborhood : public Boxing
    {
    public:
        BoxingNeighborhood(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max);
        
        virtual Solution initial() const = 0;
        virtual SolutionContainer neighbors(const Solution &solution) const = 0;
        virtual double heuristic(const Solution &solution, unsigned int iter) const = 0;
        virtual bool good(const Solution &solution) const = 0;
    };
    
    ///Boxing problem on which a local search algorithm can be applied, neighbors are items movements
    class BoxingNeighborhoodGeometry : public BoxingNeighborhood
    {
    public:
        BoxingNeighborhoodGeometry(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max);
        
        Solution initial() const override;
        SolutionContainer neighbors(const Solution &solution) const override;
        double heuristic(const Solution &solution, unsigned int iter) const override;
        bool good(const Solution &solution) const override;
    };
    
    ///Boxing problem on which a local search algorithm can be applied, neighbors are placement order rearrangements
    class BoxingNeighborhoodOrder : public BoxingNeighborhood
    {
    public:
        BoxingNeighborhoodOrder(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max);
        
        Solution initial() const override;
        SolutionContainer neighbors(const Solution &solution) const override;
        double heuristic(const Solution &solution, unsigned int iter) const override;
        bool good(const Solution &solution) const override;
    };
    
    ///Boxing problem on which a local search algorithm can be applied, neighbors are item movements with partially allowed overlaps
    class BoxingNeighborhoodGeometryOverlap : public BoxingNeighborhood
    {
    public:
        BoxingNeighborhoodGeometryOverlap(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, unsigned int desired_iter);
        
        Solution initial() const override;
        SolutionContainer neighbors(const Solution &solution) const override;
        double heuristic(const Solution &solution, unsigned int iter) const override;
        bool good(const Solution &solution) const override;
    };
}