#pragma once

#include <cstddef>
#include <mutex>
#include <string>

#include <fmt/format.h>
#include <indicators/progress_bar.hpp>

class ProgressBar {
    std::size_t total;
    std::string description;

    std::size_t counter;
    std::mutex mutex;

    indicators::ProgressBar bar{
        indicators::option::BarWidth{102},
        indicators::option::Start{"["},
        indicators::option::Fill{"="},
        indicators::option::Lead{">"},
        indicators::option::Remainder{" "},
        indicators::option::End{"]"},
        indicators::option::ShowPercentage{true},
        indicators::option::ShowElapsedTime{true},
        indicators::option::ShowRemainingTime{true},
    };

public:
    ProgressBar(std::size_t total, const std::string& description)
        : total(total), description(description), counter(0) {
        print();
    }

    void setDescription(const std::string& newDescription) {
        std::lock_guard lock(mutex);
        description = newDescription;
        print();
    }

    void update(std::size_t delta = 1) {
        std::lock_guard lock(mutex);
        counter += delta;
        print();
    }

private:
    void print() {
        bar.set_option(indicators::option::PostfixText{fmt::format("[{}/{}] {}", counter, total, description)});
        bar.set_progress(static_cast<double>(counter) / static_cast<double>(total) * 100);
    }
};
