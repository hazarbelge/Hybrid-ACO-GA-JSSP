//
// Created by Hazar Belge on 2.05.2023.
//

#include "ant_colony_optimization.h"
#include "genetic_algorithm.h"
#include <iostream>
#include <fstream>

//TEST
bool HYBRID_GA_ACO_ENABLED = true;
int NUM_RUNS = 50;
bool IS_CUSTOM_MATRIX_FOR_TEST = true; //20x5

//COMMON
int NUM_JOBS = 20;
int NUM_MACHINES = 5;

//ACO
int NUM_ANTS = 4;
int MAX_ITERATIONS = 100;
double ALPHA = 1.0;
double BETA = 2.0;
double EVAPORATION_RATE = 0.1;

//ACO OPTIMIZATION
int STAGNATION_THRESHOLD = 100;
double PHEROMONE_RESET_VALUE = 0.5;

//GA
int POPULATION_SIZE = 50;
int MAX_GENERATIONS = 200;
double CROSSOVER_RATE = 0.9;
double MUTATION_RATE = 0.2;

void write_results_to_file(double avg_run_time, double avg_initial_makeSpan, double avg_best_makeSpan, const std::vector<int>& best_schedule, const std::vector<std::vector<AntColonyOptimization::Job>> &jobs);
void print_results(double run_time, const std::vector<int> &initial_schedule, int initial_makeSpan,
                   const std::vector<int> &best_schedule, int best_makeSpan);

void print_avg_results(double avg_run_time, double avg_initial_makeSpan, double avg_best_makeSpan);

std::vector<std::vector<AntColonyOptimization::Job>> generate_jobs_matrix_list_aco(int num_jobs, int num_machines);

std::vector<std::vector<GeneticAlgorithm::Job>>
generate_jobs_matrix_list_ga(std::vector<std::vector<AntColonyOptimization::Job>> jobs_aco);

std::tuple<double, int, int, std::vector<int>>
run_ant_colony_optimization(const std::vector<std::vector<AntColonyOptimization::Job>> &jobs,
                            std::vector<int> init_schedule);

std::tuple<double, int, int, std::vector<int>>
run_genetic_algorithm(std::vector<std::vector<GeneticAlgorithm::Job>> jobs);

int main() {
    double total_run_time = 0;
    double total_initial_make_span = 0;
    double total_best_make_span = 0;

    for (int i = 0; i < NUM_RUNS; ++i) {
        std::cout << "Run " << i + 1 << std::endl;

        std::vector<std::vector<AntColonyOptimization::Job>> jobs_aco = generate_jobs_matrix_list_aco(NUM_JOBS,
                                                                                                      NUM_MACHINES);
        std::vector<std::vector<GeneticAlgorithm::Job>> jobs_ga = generate_jobs_matrix_list_ga(jobs_aco);

        std::tuple<double, int, int, std::vector<int>> ga_tuple;
        if (HYBRID_GA_ACO_ENABLED){
            ga_tuple = run_genetic_algorithm(jobs_ga);
        }
        else{
            ga_tuple = std::make_tuple(0, 0, 0, std::vector<int>());
        }
        std::tuple<double, int, int, std::vector<int>> aco_tuple = run_ant_colony_optimization(jobs_aco,
                                                                                               std::get<3>(ga_tuple));

        double run_time = std::get<0>(aco_tuple);
        int initial_makeSpan = std::get<1>(aco_tuple);
        int best_makeSpan = std::get<2>(aco_tuple);
        std::vector<int> best_schedule = std::get<3>(aco_tuple);

        write_results_to_file(run_time, initial_makeSpan, best_makeSpan, best_schedule, jobs_aco);

        total_run_time += run_time;
        total_initial_make_span += initial_makeSpan;
        total_best_make_span += best_makeSpan;
    }

    double avg_run_time = total_run_time / NUM_RUNS;
    double avg_initial_make_span = total_initial_make_span / NUM_RUNS;
    double avg_best_make_span = total_best_make_span / NUM_RUNS;

    print_avg_results(avg_run_time, avg_initial_make_span, avg_best_make_span);

    return 0;
}

std::tuple<double, int, int, std::vector<int>>
run_ant_colony_optimization(const std::vector<std::vector<AntColonyOptimization::Job>> &jobs,
                            std::vector<int> init_schedule) {
    AntColonyOptimization ant_colony(NUM_JOBS, NUM_MACHINES, jobs, NUM_ANTS, MAX_ITERATIONS, ALPHA, BETA,
                                     EVAPORATION_RATE, STAGNATION_THRESHOLD, PHEROMONE_RESET_VALUE);

    ant_colony.optimize(std::move(init_schedule));

    double run_time = ant_colony.get_run_time();
    int initial_make_span = ant_colony.get_initial_make_span();
    std::vector<int> initial_schedule = ant_colony.get_initial_schedule();
    int best_make_span = ant_colony.get_best_make_span();
    std::vector<int> best_schedule = ant_colony.get_best_schedule();

    print_results(run_time, initial_schedule, initial_make_span, best_schedule, best_make_span);

    return std::make_tuple(run_time, initial_make_span, best_make_span, best_schedule);
}

std::tuple<double, int, int, std::vector<int>>
run_genetic_algorithm(std::vector<std::vector<GeneticAlgorithm::Job>> jobs) {
    GeneticAlgorithm genetic_algorithm(NUM_JOBS, NUM_MACHINES, jobs, POPULATION_SIZE, MAX_GENERATIONS,
                                       CROSSOVER_RATE, MUTATION_RATE);

    genetic_algorithm.optimize();

    double run_time = genetic_algorithm.get_run_time();
    int initial_make_span = genetic_algorithm.get_initial_make_span();
    std::vector<int> initial_schedule = genetic_algorithm.get_initial_schedule();
    int best_make_span = genetic_algorithm.get_best_make_span();
    std::vector<int> best_schedule = genetic_algorithm.get_best_schedule();

    //print_results(run_time, initial_schedule, initial_make_span, best_schedule, best_make_span);

    return std::make_tuple(run_time, initial_make_span, best_make_span, best_schedule);
}

std::vector<std::vector<AntColonyOptimization::Job>> generate_jobs_matrix_list_aco(int num_jobs, int num_machines) {
    if (IS_CUSTOM_MATRIX_FOR_TEST) {
        return {
                {{0, 1}, {1, 4},  {2, 5},  {3, 7},  {4, 10}},
                {{1, 3}, {3, 8},  {4, 2},  {0, 7},  {2, 4}},
                {{3, 3}, {0, 9},  {1, 7},  {2, 2},  {4, 6}},
                {{2, 8}, {4, 1},  {1, 9},  {0, 4},  {3, 6}},
                {{4, 8}, {1, 7},  {3, 10}, {2, 2},  {0, 1}},
                {{2, 1}, {3, 6},  {0, 5},  {1, 6},  {4, 3}},
                {{1, 7}, {2, 2},  {3, 6},  {4, 5},  {0, 3}},
                {{3, 7}, {0, 1},  {1, 9},  {4, 10}, {2, 6}},
                {{2, 2}, {4, 8},  {0, 1},  {1, 9},  {3, 4}},
                {{4, 4}, {0, 3},  {3, 9},  {2, 7},  {1, 1}},
                {{3, 8}, {2, 3},  {1, 7},  {4, 9},  {0, 1}},
                {{0, 5}, {1, 10}, {2, 6},  {4, 8},  {3, 2}},
                {{2, 8}, {3, 7},  {4, 2},  {0, 10}, {1, 4}},
                {{4, 7}, {1, 6},  {0, 4},  {3, 8},  {2, 3}},
                {{3, 7}, {2, 2},  {1, 5},  {0, 6},  {4, 9}},
                {{1, 5}, {0, 7},  {4, 6},  {3, 8},  {2, 3}},
                {{2, 3}, {3, 7},  {1, 4},  {4, 2},  {0, 10}},
                {{4, 4}, {0, 6},  {2, 5},  {1, 8},  {3, 2}},
                {{3, 9}, {1, 2},  {4, 5},  {0, 6},  {2, 7}},
                {{2, 9}, {0, 10}, {1, 3},  {3, 7},  {4, 2}}
        };
    }

    srand(time(nullptr));

    std::vector<std::vector<AntColonyOptimization::Job>> jobs(num_jobs,
                                                              std::vector<AntColonyOptimization::Job>(num_machines));

    for (int i = 0; i < num_jobs; ++i) {
        std::vector<int> machine_list;
        for (int k = 0; k < num_machines; ++k) {
            machine_list.push_back(k);
        }
        for (int j = 0; j < num_machines; ++j) {
            int machine = machine_list[rand() % machine_list.size()];
            machine_list.erase(std::remove(machine_list.begin(), machine_list.end(), machine), machine_list.end());
            int duration = rand() % 10 + 1;
            jobs[i][j] = {machine, duration};
        }
    }

    /*
    std::cout << "Jobs: " << std::endl;
    for (int i = 0; i < num_jobs; ++i) {
        for (int j = 0; j < num_machines; ++j) {
            std::cout << "(" << jobs[i][j].machine << ", " << jobs[i][j].duration << ") ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    */

    return jobs;
}

std::vector<std::vector<GeneticAlgorithm::Job>>
generate_jobs_matrix_list_ga(std::vector<std::vector<AntColonyOptimization::Job>> jobs_aco) {
    std::vector<std::vector<GeneticAlgorithm::Job>> jobs_ga;
    for (int j = 0; j < NUM_JOBS; ++j) {
        std::vector<GeneticAlgorithm::Job> job;
        for (int k = 0; k < NUM_MACHINES; ++k) {
            GeneticAlgorithm::Job temp_job{};
            temp_job.machine = jobs_aco[j][k].machine;
            temp_job.duration = jobs_aco[j][k].duration;
            job.push_back(temp_job);
        }
        jobs_ga.push_back(job);
    }
    return jobs_ga;
}

void print_results(double run_time, const std::vector<int> &initial_schedule, int initial_makeSpan,
                   const std::vector<int> &best_schedule, int best_makeSpan) {
    std::cout << "" << std::endl;
    std::cout << "---------------- Results ------------------------" << std::endl;

    std::cout << "Initial Schedule: ";
    for (const auto &job: initial_schedule) {
        std::cout << job << " ";
    }
    std::cout << std::endl;
    std::cout << "Initial MakeSpan: " << initial_makeSpan << std::endl;
    std::cout << std::endl;
    std::cout << "Best Schedule: ";
    for (const auto &job: best_schedule) {
        std::cout << job << " ";
    }
    std::cout << std::endl;
    std::cout << "Best MakeSpan: " << best_makeSpan << std::endl;

    std::cout << std::endl;
    std::cout << "Improvement: " << (double) (initial_makeSpan - best_makeSpan) / initial_makeSpan * 100 << "%"
              << std::endl;
    std::cout << "Run Time: " << run_time / 1000.0 << " seconds" << std::endl;
}

void print_avg_results(double avg_run_time, double avg_initial_makeSpan, double avg_best_makeSpan) {
    std::cout << "---------------- Average Results ------------------------" << std::endl;

    std::cout << "Average Initial MakeSpan: " << avg_initial_makeSpan << std::endl;
    std::cout << "Average Best MakeSpan: " << avg_best_makeSpan << std::endl;

    std::cout << std::endl;
    std::cout << "Average Improvement: "
              << (double) (avg_initial_makeSpan - avg_best_makeSpan) / avg_initial_makeSpan * 100 << "%" << std::endl;
    std::cout << "Average Run Time: " << avg_run_time / 1000.0 << " seconds" << std::endl;
}

void write_results_to_file(double avg_run_time, double avg_initial_makeSpan, double avg_best_makeSpan, const std::vector<int>& best_schedule, const std::vector<std::vector<AntColonyOptimization::Job>> &jobs) {
    std::ofstream results_file;
    results_file.open("../results.txt", std::ios::app);
    results_file << "NUM_JOBS:" << NUM_JOBS;
    results_file << ",NUM_MACHINES:" << NUM_MACHINES;
    results_file << ",AVG_RUN_TIME:" << avg_run_time;
    results_file << ",AVG_INITIAL_MAKE_SPAN:" << avg_initial_makeSpan;
    results_file << ",AVG_BEST_MAKE_SPAN:" << avg_best_makeSpan;
    results_file << ",SCHEDULE:[";
    for (const auto &job: best_schedule) {
        results_file << job << ",";
    }
    results_file.seekp(-1, std::ios_base::end);
    results_file << "],JOBS:";
    for (const auto &job: jobs) {
        for (const auto &task: job) {
            results_file << "(";
            results_file << task.machine << ":" << task.duration;
            results_file << ")";
        }
        results_file << ",";
    }
    results_file << std::endl;
    results_file.close();
}

