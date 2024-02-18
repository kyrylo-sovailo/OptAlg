#pragma once
#include <algorithm>
#include <limits>
#include <set>
#include <vector>
#include <time.h>

namespace opt
{
    /**
    Solves an optimization problem by greedy method
    
    Problem class should satisfy requirements:
     - Problem::Element be an element
     - Problem::Solution be a solution (and also behaves like a set)
     - Problem::ElementContainer be a container for elements (and also can be iterated through)
     
     - ElementContainer Problem::elements() returns all available elements
     - bool Problem::can_join(Solution solution, Element element) returns if it is possible to add element to solution
     - Solution Problem::join(Solution solution, Element element) adds element to solution
     - double Problem::weight(Element element) returns weight of the element
    */
    template <class Problem> typename Problem::Solution greedy(
        const Problem &problem,
        std::vector<typename Problem::Solution> *log,
        double *timer)
    {
        //Start clock
        clock_t start = clock();
        
        //Define types
        typedef typename Problem::Element Element;
        typedef typename Problem::Solution Solution;
        typedef typename Problem::ElementContainer Container;
        struct WeightedElement
        {
            const Element *element;
            double weight;
            
            WeightedElement(const Element &element, double weight) : element(&element), weight(weight) {}
            bool operator<(const WeightedElement &other) const { return weight < other.weight; }
        };
        
        //Get elements
        const Container &elements = problem.elements();
        
        //Weight elements
        std::vector<WeightedElement> weighted_elements;
        weighted_elements.reserve(elements.size());
        for (auto element = elements.cbegin(); element != elements.cend(); element++)
        {
            weighted_elements.push_back(WeightedElement(*element, problem.weight(*element)));
        }
        
        //Sort weighted elements by weight
        std::sort(weighted_elements.begin(), weighted_elements.end());
        
        //Create empty set
        Solution solution;
        if (log != nullptr) log->push_back(solution);
        
        //Try to add every element
        for (auto element = weighted_elements.crbegin(); element != weighted_elements.crend(); element++)
        {
            if (problem.can_join(solution, *element->element))
            {
                solution = problem.join(std::move(solution), *element->element);
                if (log != nullptr) log->push_back(solution);
            }
        }
        
        //Return
        clock_t finish = clock();
        if (timer != nullptr) *timer = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
        return solution;
    }
}