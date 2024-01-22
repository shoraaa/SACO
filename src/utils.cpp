/**
 * @author: Rafał Skinderowicz (rafal.skinderowicz@us.edu.pl)
 */
#include <fstream>
#include <cmath>
#include <sstream>
#include <ctime>
#include <iostream>
#include <chrono>
#include <iomanip>
#include "utils.h"
#include "json.hpp"

#define FMT_HEADER_ONLY
#include "include/fmt/format.h"


std::ostream & operator<<(std::ostream &out, const Timer &timer) {
    return out << timer.get_elapsed_seconds();
}

nlohmann::json &get_best_known_solutions() {
    static nlohmann::json solutions;
    return solutions;
}

void load_best_known_solutions(const std::string &path) {
    std::ifstream in(path);
    auto &db = get_best_known_solutions();

    if (in.is_open()) {
        in >> db;
        in.close();
    }
}


double get_best_known_value(const std::string &instance_name,
                            double default_value = 0.0) {
    const auto &db = get_best_known_solutions();
    auto it = db.find(instance_name);
    return it != db.end() ? it.value().get<double>() : default_value;
}


double sample_mean(const std::vector<double> &vec) {
    assert(!vec.empty());
    return std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}


double sample_stdev(const std::vector<double> &vec) {
    assert(vec.size() >= 2);
    auto mean = sample_mean(vec);
    double sum = 0;
    for (auto &x : vec) {
        auto diff = x - mean;
        sum += diff * diff;
    }
    return std::sqrt(sum / (vec.size() - 1));
}


/*
 * Returns a string with the current date & time,
 * e.g. 2021-12-31 15:45:59
 */
std::string get_current_datetime_string(const char *date_sep,
                                        const char *time_sep,
                                        const char *between_sep,
                                        bool include_ns) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}
