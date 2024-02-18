#pragma once
#include <algorithm>
#include <limits>
#include <set>
#include <vector>
#include <time.h>

namespace opt
{
    /**
    Solves an optimization problem by heuristic neighborhood optimization
    
    Problem class should satisfy requirements:
     - Problem::Solution be a feasible solution
     - Problem::SolutionContainer be a set of solutions
     
     - Solution Problem::initial() returns initial feasible solution
     - SolutionContainer Problem::neighbors(Solution solution) returns solution neighbors
     - double Problem::heuristic(Solution solution, unsigned int iter) returns solution heuristics
     - bool Problem::good(Solution solution) returns if solution is good enough and algorithm can terminate
    */
    template <class Problem> typename Problem::Solution neighborhood(
        const Problem &problem,
        unsigned int iter_max,
        double time_max,
        std::vector<typename Problem::Solution> *log,
        double *timer)
    {        
        //Define types
        typedef typename Problem::Solution Solution;
        typedef typename Problem::SolutionContainer Container;
        
        //Start clock
        clock_t start = clock();
        clock_t clock_max = static_cast<clock_t>(time_max * CLOCKS_PER_SEC);
        unsigned int iter = 0;
        
        //Iterate
        Solution solution = problem.initial();
        if (log != nullptr) log->push_back(solution);
        while (true)
        {
            //Get heuristic
            double solution_heuristic = problem(solution, iter);
                    
            //Get neighborhood
            Container neighbors = problem.neighbors(solution, iter);
            
            //Search best neighbor
            auto best_neighbor = neighbors.cend();
            double best_neighbor_heuristic = -std::numeric_limits<double>::infinity();
            for (auto neighbor = neighbors.cbegin(); neighbor != neighbors.cend(); neighbor++)
            {
                double neighbor_heuristic = problem(*neighbor, iter);
                if (neighbor_heuristic > solution_heuristic && neighbor_heuristic > best_neighbor_heuristic)
                {
                    best_neighbor = neighbor;
                    best_neighbor_heuristic = neighbor_heuristic;
                }
            }
            
            //Go to best neighbor
            if (best_neighbor != neighbors.cend())
            {
                solution = *best_neighbor;
                solution_heuristic = best_neighbor_heuristic;
                if (log != nullptr) log->push_back(solution);
            }
            
            //Exit
            if (problem.good(solution))
            {
                if (best_neighbor == neighbors.cend()) break;   //No better neighbor
                else if (iter >= iter_max) break;               //Maximum iteration reached
                else if (clock() - start >= clock_max) break;   //Maximum time reached
            }
            iter++;
        }
        
        //Return
        clock_t finish = clock();
        if (timer != nullptr) *timer = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
        return solution;
    }
}