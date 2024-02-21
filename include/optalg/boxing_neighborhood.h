#pragma once
#include "boxing.h"
#include <random>
#include <vector>

namespace opt
{
    ///Boxing problem on which a local search algorithm can be applied, neighbors are items movements
    class BoxingNeighborhoodGeometry : public Boxing
    {
    protected:
        unsigned int _window;
    
    public:
        BoxingNeighborhoodGeometry(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, unsigned int seed,
            unsigned int window);
        
        //Implementing neighborhood requirements
        typedef std::vector<Box> Solution;
        typedef std::vector<Solution> SolutionContainer;
        Solution initial(unsigned int seed) const;
        SolutionContainer neighbors(const Solution &solution, std::default_random_engine &engine, unsigned int id = 0, unsigned int nthreads = 1) const;
        double heuristic(const Solution &solution, unsigned int iter) const;
        bool good(const Solution &solution) const;

        //Getting specific data
        std::vector<Box> get_boxes(const Solution &solution) const;
    };
    
    ///Boxing problem on which a local search algorithm can be applied, neighbors are placement order rearrangements
    class BoxingNeighborhoodOrder : public Boxing
    {
    protected:
        unsigned int _window;
    
    public:
        BoxingNeighborhoodOrder(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, unsigned int seed,
            unsigned int window);
        
        //Implementing neighborhood requirements
        typedef std::vector<const Rectangle*> Solution;
        typedef std::vector<Solution> SolutionContainer;
        Solution initial(unsigned int seed) const;
        SolutionContainer neighbors(const Solution &solution, std::default_random_engine &engine, unsigned int id = 0, unsigned int nthreads = 1) const;
        double heuristic(const Solution &solution, unsigned int iter) const;
        bool good(const Solution &solution) const;

        //Getting specific data
        std::vector<Box> get_boxes(const Solution &solution) const;
    };
    
    ///Boxing problem on which a local search algorithm can be applied, neighbors are item movements with partially allowed overlaps
    class BoxingNeighborhoodGeometryOverlap : public Boxing
    {
    protected:
        unsigned int _window;
        unsigned int _desired_iter;

        unsigned int _overlapping_area(const BoxedRectangle &a, const BoxedRectangle &b) const;
    
    public:
        BoxingNeighborhoodGeometryOverlap(unsigned int box_size, unsigned int item_number, unsigned int item_size_min, unsigned int item_size_max, unsigned int seed,
            unsigned int window, unsigned int desired_iter);
        
        //Implementing neighborhood requirements
        typedef std::vector<Box> Solution;
        typedef std::vector<Solution> SolutionContainer;
        Solution initial(unsigned int seed) const;
        SolutionContainer neighbors(const Solution &solution, std::default_random_engine &engine, unsigned int id = 0, unsigned int nthreads = 1) const;
        double heuristic(const Solution &solution, unsigned int iter) const;
        bool good(const Solution &solution) const;

        //Getting specific data
        std::vector<Box> get_boxes(const Solution &solution) const;
    };
}