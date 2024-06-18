#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <uspto/config.h>
#include <uspto/index.h>
#include <uspto/patents.h>
#include <uspto/searcher.h>

double getMean(const std::vector<double>& values) {
    return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
}

double getMedian(const std::vector<double>& values) {
    auto valuesSorted = values;
    std::sort(valuesSorted.begin(), valuesSorted.end());

    auto length = values.size();
    if (length == 0) {
        return 0;
    } else if (length % 2 == 0) {
        return (values[length / 2] + values[length / 2 - 1]) / 2;
    } else {
        return values[length / 2];
    }
}

int main() {
    spdlog::info("Creating patent reader");
    PatentReader patentReader;

    spdlog::info("Creating search index");
    auto searchIndex = getSearchIndex(
        patentReader,
        getValidationDataDirectory() / "neighbors_small.csv",
        -1,
        false);

    Searcher searcher(searchIndex);

    std::vector<std::filesystem::path> files;
    std::copy(
        std::filesystem::directory_iterator(getProjectDirectory() / "tests" / "queries"),
        std::filesystem::directory_iterator(),
        std::back_inserter(files));
    std::sort(files.begin(), files.end());

    std::vector<double> percentages;
    std::vector<double> durations;

    for (const auto& file : files) {
        std::ifstream stream(file);
        auto json = nlohmann::json::parse(stream);

        auto query = json["query"].get<std::string>();

        ankerl::unordered_dense::set<std::string> expectedResults;
        for (const auto& value : json["results"]) {
            expectedResults.emplace(value.get<std::string>());
        }

        auto startTime = std::chrono::high_resolution_clock::now();
        auto results = searcher.search(query);
        auto endTime = std::chrono::high_resolution_clock::now();

        int matches = 0;
        for (const auto& patent : expectedResults) {
            if (results.contains(patent)) {
                ++matches;
            }
        }

        double percentage =
                matches == expectedResults.size()
                    ? 100
                    : static_cast<double>(matches) / static_cast<double>(expectedResults.size()) * 100;
        double durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

        percentages.emplace_back(percentage);
        durations.emplace_back(durationNs);

        spdlog::info(
            "{}: {}/{} ({:.2f}%) in {:.3f} ms",
            file.filename().c_str(),
            matches,
            expectedResults.size(),
            percentage,
            durationNs / 1e6);
    }

    spdlog::info(
        "Total time taken: {:.3f} ms | Mean: {:.2f}% in {:.3f} ms | Median: {:.2f}% in {:.3f} ms",
        std::accumulate(durations.begin(), durations.end(), 0.0) / 1e6,
        getMean(percentages),
        getMean(durations) / 1e6,
        getMedian(percentages),
        getMedian(durations) / 1e6);

    return 0;
}
