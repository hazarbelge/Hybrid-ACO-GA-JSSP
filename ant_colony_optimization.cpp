//
// Created by Hazar Belge on 2.05.2023.
//

#include "ant_colony_optimization.h"
#include <iostream>

AntColonyOptimization::AntColonyOptimization(int num_jobs, int num_machines, const std::vector<std::vector<Job>> &jobs,
                                             int num_ants, int max_iterations, double alpha, double beta,
                                             double evaporation_rate, int stagnation_threshold,
                                             double pheromone_reset_value)
        : num_jobs_(num_jobs), num_machines_(num_machines), jobs_(jobs), num_ants_(num_ants),
          max_iterations_(max_iterations), alpha_(alpha), beta_(beta), evaporation_rate_(evaporation_rate),
          stagnation_threshold_(stagnation_threshold), pheromone_reset_value_(pheromone_reset_value),
          num_stagnant_iterations_(0) {
    initialize_pheromone_matrix();
    ants_.resize(num_ants_);
    best_ant_.make_span = std::numeric_limits<int>::max();
    run_time_ = 0.0;
}

void AntColonyOptimization::initialize_pheromone_matrix() {
    pheromone_matrix_.resize(num_jobs_, std::vector<double>(num_jobs_, 1.0));
}

void AntColonyOptimization::optimize(const std::vector<int>& init_schedule) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < max_iterations_; ++iter) {
        construct_solutions();
        local_search();

        for (const Ant &ant: ants_) {
            if (ant.make_span < best_ant_.make_span) {
                best_ant_ = ant;
                elitist_ant_ = best_ant_;
                num_stagnant_iterations_ = 0;
            }
        }

        if (iter == 0) {
            initial_ant_ = best_ant_;

            if(!init_schedule.empty()) {
                best_ant_.schedule = init_schedule;
                best_ant_.make_span = calculate_make_span(init_schedule);
                elitist_ant_ = best_ant_;

                for (size_t i = 0; i < init_schedule.size() - 1; ++i) {
                    int from_job = init_schedule[i];
                    int to_job = init_schedule[i + 1];
                    pheromone_matrix_[from_job][to_job] += 1.0 / calculate_make_span(init_schedule);
                }
            }
        }

        if (++num_stagnant_iterations_ >= stagnation_threshold_) {
            reset_pheromone_levels(pheromone_reset_value_);
            num_stagnant_iterations_ = 0;
        } else {
            update_pheromones();
        }

        add_pheromone_from_elitist_ant();

        /*
        std::cout << "Iteration " << iter + 1 << " - Best MakeSpan: " << best_ant_.make_span << std::endl;
        std::cout << "Best Schedule: ";
        for (const int& job : best_ant_.schedule) {
            std::cout << job << " ";
        }
        std::cout << std::endl;
        */
    }
    auto end = std::chrono::high_resolution_clock::now();
    run_time_ = (double) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void AntColonyOptimization::reset_pheromone_levels(double value) {
    for (std::vector<double>& row : pheromone_matrix_) {
        for (double& pheromone : row) {
            pheromone = value;
        }
    }
}

void AntColonyOptimization::add_pheromone_from_elitist_ant() {
    for (size_t i = 0; i < elitist_ant_.schedule.size() - 1; ++i) {
        int from_job = elitist_ant_.schedule[i];
        int to_job = elitist_ant_.schedule[i + 1];
        pheromone_matrix_[from_job][to_job] += 1.0 / elitist_ant_.make_span;
    }
}

void AntColonyOptimization::construct_solutions() {
    std::vector<int> jobs_list(num_jobs_);
    for (int i = 0; i < num_jobs_; ++i) {
        jobs_list[i] = i;
    }

    for (Ant &ant: ants_) {
        std::vector<int> remaining_jobs = jobs_list;
        ant.schedule.clear();
        ant.schedule.reserve(num_jobs_);

        while (!remaining_jobs.empty()) {
            int current_job = ant.schedule.empty() ? 0 : ant.schedule.back();
            std::vector<double> probabilities;

            for (const int &job: remaining_jobs) {
                double pheromone = pheromone_matrix_[current_job][job];
                double heuristic = 1.0 / (1.0 + calculate_make_span(ant.schedule) +
                                          jobs_[job][current_job % num_machines_].duration);
                for (int i = 0; i < alpha_; ++i) {
                    pheromone *= pheromone;
                }
                for (int i = 0; i < beta_; ++i) {
                    heuristic *= heuristic;
                }
                probabilities.push_back(pheromone * heuristic);
            }

            double cum_prob = 0.0;
            for (double probability: probabilities) {
                cum_prob += probability;
            }

            double rand_num = static_cast<double>(rand()) / RAND_MAX * cum_prob;

            int selected_index = -1;
            double cum_prob_so_far = 0.0;
            for (int i = 0; i < (int) probabilities.size(); ++i) {
                cum_prob_so_far += probabilities[i];
                if (rand_num <= cum_prob_so_far) {
                    selected_index = i;
                    break;
                }
            }

            if (selected_index == -1) {
                selected_index = (int) probabilities.size() - 1;
            }

            int selected_job = remaining_jobs[selected_index];

            ant.schedule.push_back(selected_job);
            remaining_jobs.erase(remaining_jobs.begin() + selected_index);
        }

        ant.make_span = calculate_make_span(ant.schedule);
    }
}

void AntColonyOptimization::local_search() {
    auto get_duration = [this](int job1, int job2) {
        if (job1 < 0 || job2 < 0) {
            return 0;
        }
        int duration_sum = 0;
        for (int k = job1; k <= job2; ++k) {
            duration_sum += jobs_[k][0].duration;
        }
        return duration_sum;
    };

    for (Ant &ant: ants_) {
        bool improvement = false;
        while (!improvement) {
            improvement = true;
            for (size_t i = 0; i < ant.schedule.size() - 1; ++i) {
                for (size_t j = i + 1; j < ant.schedule.size() - 1; ++j) {
                    int before_i = i > 0 ? ant.schedule[i - 1] : -1;
                    int before_j = j > 0 ? ant.schedule[j - 1] : -1;
                    int after_i = i < static_cast<int>(ant.schedule.size()) - 1 ? ant.schedule[i + 1] : -1;
                    int after_j = j < static_cast<int>(ant.schedule.size()) - 1 ? ant.schedule[j + 1] : -1;

                    int old_make_span = std::max(
                            {get_duration(before_i, ant.schedule[i]), get_duration(ant.schedule[i], after_i),
                             get_duration(before_j, ant.schedule[j]), get_duration(ant.schedule[j], after_j)});
                    int new_make_span = std::max(
                            {get_duration(before_i, ant.schedule[j]), get_duration(ant.schedule[j], after_i),
                             get_duration(before_j, ant.schedule[i]), get_duration(ant.schedule[i], after_j)});

                    int delta = new_make_span - old_make_span;

                    if (delta > 0) {
                        std::swap(ant.schedule[i], ant.schedule[j]);
                        ant.make_span += delta;
                        improvement = false;
                    }
                }
            }
        }
    }
}

void AntColonyOptimization::update_pheromones() {
    for (std::vector<double> &row: pheromone_matrix_) {
        for (double &pheromone: row) {
            pheromone *= (1.0 - evaporation_rate_);
        }
    }

    for (const Ant &ant: ants_) {
        for (size_t i = 0; i < ant.schedule.size() - 1; ++i) {
            pheromone_matrix_[ant.schedule[i]][ant.schedule[i + 1]] += 1.0 / ant.make_span;
        }
    }
}

int AntColonyOptimization::calculate_make_span(const std::vector<int> &schedule) {
    std::vector<int> machine_times(num_machines_, 0);
    std::vector<int> job_times(num_jobs_, 0);

    for (const int &job: schedule) {
        for (const Job &task: jobs_[job]) {
            int machine = task.machine;
            int duration = task.duration;

            int start_time = std::max(machine_times[machine], job_times[job]);
            machine_times[machine] = start_time + duration;
            job_times[job] = machine_times[machine];
        }
    }

    return *std::max_element(machine_times.begin(), machine_times.end());
}

double AntColonyOptimization::get_run_time() const {
    return run_time_;
}

int AntColonyOptimization::get_initial_make_span() const {
    return initial_ant_.make_span;
}

std::vector<int> AntColonyOptimization::get_initial_schedule() const {
    return initial_ant_.schedule;
}

int AntColonyOptimization::get_best_make_span() const {
    return best_ant_.make_span;
}

std::vector<int> AntColonyOptimization::get_best_schedule() const {
    return best_ant_.schedule;
}
