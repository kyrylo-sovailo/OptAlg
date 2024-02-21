#pragma once
#include <cmath>
#include <limits>
#include <vector>
#include <thread>
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
        Problem &problem,
        unsigned int iter_max,
        double time_max,
        std::vector<typename Problem::Solution> *log,
        double *timer)
    {        
        //Define types
        typedef typename Problem::Solution Solution;
        typedef typename Problem::SolutionContainer Container;
        struct Thread
        {
            typename Container::const_iterator begin, end;
            const Solution *solution;
            double heuristic;
            std::thread thread;
        };
        
        //Start clock and threads
        const unsigned int nthreads = std::thread::hardware_concurrency();
        std::vector<Thread> threads(nthreads);
        const bool clock_limited = std::isfinite(time_max);
        const clock_t clock_max = clock_limited ? static_cast<clock_t>(time_max * CLOCKS_PER_SEC) : 0;
        const clock_t start = clock();
        
        //Iterate
        Solution solution = problem.initial();
        if (log != nullptr) log->push_back(solution);
        for (unsigned int iter = 0;; iter++)
        {
            //Get heuristic
            double solution_heuristic = problem.heuristic(solution, iter);
                    
            //Get neighborhood
            Container neighbors = problem.neighbors(solution);
            
            //Search best neighbors
            for (unsigned int id = 0; id < threads.size(); id++)
            {
                //TODO: generalize not only for random iterators
                threads[id].begin = neighbors.begin() + (neighbors.size() * id / nthreads);
                threads[id].end = neighbors.begin() + (neighbors.size() * (id + 1) / nthreads);
                threads[id].solution = nullptr;
                threads[id].heuristic = std::numeric_limits<double>::infinity();
                threads[id].thread = std::thread(
                [iter, solution_heuristic, &problem](Thread *thread)
                {
                    for (auto neighbor = thread->begin; neighbor != thread->end; neighbor++)
                    {
                        double neighbor_heuristic = problem.heuristic(*neighbor, iter);
                        if (neighbor_heuristic < solution_heuristic && neighbor_heuristic < thread->heuristic)
                        {
                            thread->solution = &(*neighbor);
                            thread->heuristic = neighbor_heuristic;
                        }
                    }
                }, &threads[id]);
            }

            //Search best neighbor
            const Solution *best_neighbor = nullptr;
            double best_neighbor_heuristic = std::numeric_limits<double>::infinity();
            for (unsigned int id = 0; id < threads.size(); id++)
            {
                threads[id].thread.join();
                if (threads[id].heuristic < solution_heuristic && threads[id].heuristic < best_neighbor_heuristic)
                {
                    best_neighbor = threads[id].solution;
                    best_neighbor_heuristic = threads[id].heuristic;
                }
            }

            //Go to best neighbor
            if (best_neighbor != nullptr)
            {
                solution = *best_neighbor;
                if (log != nullptr) log->push_back(solution);
            }
            
            //Exit
            if (problem.good(solution))
            {
                if (best_neighbor == nullptr) break;                            //No better neighbor
                else if (iter >= iter_max) break;                               //Maximum iteration reached
                else if (clock_limited && clock() - start >= clock_max) break;  //Maximum time reached
            }
        }
        
        //Return
        clock_t finish = clock();
        if (timer != nullptr) *timer = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
        return solution;
    }
}