//
// Created by Hazar Belge on 3.05.2023.
//

#include "genetic_algorithm.h"
#include <iostream>

GeneticAlgorithm::GeneticAlgorithm(int num_jobs, int num_machines, const std::vector<std::vector<Job>> &jobs,
                                   int population_size, int max_generations, double crossover_rate,
                                   double mutation_rate)
        : num_jobs_(num_jobs), num_machines_(num_machines), jobs_(jobs), population_size_(population_size),
          max_generations_(max_generations), crossover_rate_(crossover_rate), mutation_rate_(mutation_rate) {
    srand(time(nullptr));
}

void GeneticAlgorithm::optimize() {
    auto start = std::chrono::high_resolution_clock::now();

    initialize_population();
    calculate_make_spans();

    initial_make_span_ = *std::min_element(make_spans_.begin(), make_spans_.end());
    initial_schedule_ = population_[std::min_element(make_spans_.begin(), make_spans_.end()) - make_spans_.begin()];

    for (int generation = 0; generation < max_generations_; ++generation) {
        selection();
        crossover();
        mutation();
        calculate_make_spans();
    }

    auto end = std::chrono::high_resolution_clock::now();
    run_time_ = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}

void GeneticAlgorithm::initialize_population() {
    population_.resize(population_size_, std::vector<int>(num_jobs_));

    for (int i = 0; i < population_size_; ++i) {
        for (int j = 0; j < num_jobs_; ++j) {
            population_[i][j] = j;
        }
        //shuffle population without using random library
        for (int j = 0; j < num_jobs_; ++j) {
            int random_index = rand() % num_jobs_;
            std::swap(population_[i][j], population_[i][random_index]);
        }
    }
}

void GeneticAlgorithm::calculate_make_spans() {
    make_spans_.resize(population_size_);
    best_make_span_ = INT32_MAX;

    for (int i = 0; i < population_size_; ++i) {
        int make_span = calculate_make_span(population_[i]);
        make_spans_[i] = make_span;

        if (make_span < best_make_span_) {
            best_make_span_ = make_span;
            best_schedule_ = population_[i];
        }
    }
}

void GeneticAlgorithm::selection() {
    std::vector<std::vector<int>> new_population(population_size_, std::vector<int>(num_jobs_));

    for (int i = 0; i < population_size_; ++i) {
        int parent1 = rand() % population_size_;
        int parent2 = rand() % population_size_;

        if (make_spans_[parent1] < make_spans_[parent2]) {
            new_population[i] = population_[parent1];
        } else {
            new_population[i] = population_[parent2];
        }
    }

    population_ = new_population;
}

void GeneticAlgorithm::crossover() {
    for (int i = 0; i < population_size_; i += 2) {
        if (static_cast<double>(rand()) / RAND_MAX < crossover_rate_) {
            int crossover_point = rand() % (num_jobs_ - 1) + 1;

            for (int j = crossover_point; j < num_jobs_; ++j) {
                int job1 = population_[i][j];
                int job2 = population_[i + 1][j];

                auto iter1 = std::find(population_[i].begin(), population_[i].begin() + crossover_point, job2);
                auto iter2 = std::find(population_[i + 1].begin(), population_[i + 1].begin() + crossover_point, job1);

                std::swap(*iter1, population_[i][j]);
                std::swap(*iter2, population_[i + 1][j]);
            }
        }
    }
}

void GeneticAlgorithm::mutation() {
    for (int i = 0; i < population_size_; ++i) {
        if (static_cast<double>(rand()) / RAND_MAX < mutation_rate_) {
            int mutation_point1 = rand() % num_jobs_;
            int mutation_point2 = rand() % num_jobs_;

            std::swap(population_[i][mutation_point1], population_[i][mutation_point2]);
        }
    }
}

int GeneticAlgorithm::calculate_make_span(const std::vector<int> &schedule) const {
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

std::vector<int> GeneticAlgorithm::get_best_schedule() const {
    return best_schedule_;
}

int GeneticAlgorithm::get_best_make_span() const {
    return best_make_span_;
}

double GeneticAlgorithm::get_run_time() const {
    return run_time_;
}

int GeneticAlgorithm::get_initial_make_span() const {
    return initial_make_span_;
}

std::vector<int> GeneticAlgorithm::get_initial_schedule() const {
    return initial_schedule_;
}