//
// Created by Hazar Belge on 2.05.2023.
//

#ifndef ANT_COLONY_OPTIMIZATION_H
#define ANT_COLONY_OPTIMIZATION_H

#include <vector>

class AntColonyOptimization {
public:
    struct Job {
        int machine;
        int duration;
    };

    struct Ant {
        std::vector<int> schedule;
        int make_span;
    };

    AntColonyOptimization(
            int num_jobs,
            int num_machines,
            const std::vector<std::vector<Job>> &jobs,
            int num_ants,
            int max_iterations,
            double alpha,
            double beta,
            double evaporation_rate,
            int stagnation_threshold,
            double pheromone_reset_value
    );

    void optimize(const std::vector<int> &init_schedule);

    int get_initial_make_span() const;

    std::vector<int> get_initial_schedule() const;

    int get_best_make_span() const;

    std::vector<int> get_best_schedule() const;

    double get_run_time() const;

private:
    int num_jobs_;
    int num_machines_;
    std::vector<std::vector<Job>> jobs_;
    int num_ants_;
    int max_iterations_;
    double alpha_;
    double beta_;
    double evaporation_rate_;
    std::vector<std::vector<double>> pheromone_matrix_;
    std::vector<Ant> ants_;
    Ant initial_ant_;
    Ant best_ant_;

    double run_time_;

    int stagnation_threshold_;
    int num_stagnant_iterations_;
    double pheromone_reset_value_;
    Ant elitist_ant_;

    void initialize_pheromone_matrix();

    void reset_pheromone_levels(double value);

    void add_pheromone_from_elitist_ant();

    void construct_solutions();

    void local_search();

    void update_pheromones();

    int calculate_make_span(const std::vector<int> &schedule);
};

#endif // ANT_COLONY_OPTIMIZATION_H