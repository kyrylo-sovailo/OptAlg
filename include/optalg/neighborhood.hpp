#pragma once
#include <cmath>
#include <limits>
#include <random>
#include <vector>
#include <time.h>
#ifdef NDEBUG
    #include <thread>
#endif

namespace opt
{
    /**
    Solves an optimization problem by heuristic neighborhood optimization
    
    Problem class should satisfy requirements:
     - Problem::Solution be a feasible solution
     - Problem::SolutionContainer be a set of solutions
     
     - Solution Problem::initial() returns initial feasible solution
     - SolutionContainer Problem::neighbors(Solution solution, int id, int threads) returns solution neighbors
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
            unsigned int id;
            Solution solution;
            double heuristic;
            std::default_random_engine engine;
            #ifdef NDEBUG
                std::thread thread;
            #endif
        };
        
        //Start threads
        #ifdef NDEBUG
            const unsigned int nthreads = std::thread::hardware_concurrency();
        #else
            const unsigned int nthreads = 1;
        #endif
        std::vector<Thread> threads(nthreads);
        for (unsigned int id = 0; id < threads.size(); id++) threads[id].engine.seed(id);

        //Start clock
        const bool clock_limited = std::isfinite(time_max);
        const clock_t clock_max = clock_limited ? static_cast<clock_t>(time_max * CLOCKS_PER_SEC) : 0;
        const clock_t start = clock();
        
        //Iterate
        Solution solution = problem.initial(0);
        if (log != nullptr) log->push_back(solution);
        for (unsigned int iter = 0;; iter++)
        {
            //Get heuristic
            double solution_heuristic = problem.heuristic(solution, iter);
                    
            //Start threads
            for (unsigned int id = 0; id < threads.size(); id++)
            {
                threads[id].id = id;
                threads[id].heuristic = std::numeric_limits<double>::infinity();
                #ifdef NDEBUG
                threads[id].thread = std::thread(
                [nthreads, iter, solution_heuristic, &solution, &problem](Thread *thread)
                #else
                Thread *thread = &threads[0];
                #endif
                {
                    //Get neighborhood
                    Container neighbors = problem.neighbors(solution, thread->engine, thread->id, nthreads);

                    //Search for best neighbor
                    for (auto neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++)
                    {
                        double neighbor_heuristic = problem.heuristic(*neighbor, iter);
                        if (neighbor_heuristic < solution_heuristic && neighbor_heuristic < thread->heuristic)
                        {
                            thread->solution = *neighbor;
                            thread->heuristic = neighbor_heuristic;
                        }
                    }
                }
                #ifdef NDEBUG
                , &threads[id]);
                #endif
            }

            //Search best neighbor
            Solution best_neighbor;
            double best_neighbor_heuristic = std::numeric_limits<double>::infinity();
            for (unsigned int id = 0; id < threads.size(); id++)
            {
                #ifdef NDEBUG
                    threads[id].thread.join();
                #endif
                if (threads[id].heuristic < solution_heuristic && threads[id].heuristic < best_neighbor_heuristic)
                {
                    best_neighbor = threads[id].solution;
                    best_neighbor_heuristic = threads[id].heuristic;
                }
            }

            //Go to best neighbor
            if (std::isfinite(best_neighbor_heuristic))
            {
                solution = best_neighbor;
                if (log != nullptr) log->push_back(solution);
            }
            
            //Exit
            if (problem.good(solution))
            {
                if (!std::isfinite(best_neighbor_heuristic)) break;             //No better neighbor
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