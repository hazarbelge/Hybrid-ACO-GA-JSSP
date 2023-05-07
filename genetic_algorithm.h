//
// Created by Hazar Belge on 3.05.2023.
//

#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include <vector>

class GeneticAlgorithm {
public:
    struct Job {
        int machine;
        int duration;
    };

    GeneticAlgorithm(
            int num_jobs,
            int num_machines,
            const std::vector<std::vector<Job>> &jobs,
            int population_size,
            int max_generations,
            double crossover_rate,
            double mutation_rate
    );

    void optimize();

    int get_initial_make_span() const;

    std::vector<int> get_initial_schedule() const;

    int get_best_make_span() const;

    std::vector<int> get_best_schedule() const;

    double get_run_time() const;

private:
    int num_jobs_;
    int num_machines_;
    std::vector<std::vector<Job>> jobs_;
    int population_size_;
    int max_generations_;
    double crossover_rate_;
    double mutation_rate_;

    std::vector<std::vector<int>> population_;
    std::vector<int> make_spans_;
    int best_make_span_;
    std::vector<int> best_schedule_;
    int initial_make_span_;
    std::vector<int> initial_schedule_;
    double run_time_;

    void initialize_population();

    void calculate_make_spans();

    void selection();

    void crossover();

    void mutation();

    int calculate_make_span(const std::vector<int> &schedule) const;
};

#endif // GENETIC_ALGORITHM_H
