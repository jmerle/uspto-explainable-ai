#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <BS_thread_pool.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <uspto/csv.h>
#include <uspto/generators.h>
#include <uspto/grafana.h>
#include <uspto/index.h>
#include <uspto/patents.h>
#include <uspto/progress.h>
#include <uspto/searcher.h>

class Timer {
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

public:
    double elapsedSeconds() const {
        return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
    }
};

struct Task {
    std::size_t id;
    std::string publicationNumber;
    std::vector<std::string> targets;

    std::string bestQuery = "ti:device";
    double bestScore = 0.0;
    std::string bestQueryGenerator = "null";

    Task(std::size_t id, const std::string& publicationNumber, const std::vector<std::string>& targets)
        : id(id),
          publicationNumber(publicationNumber),
          targets(targets) {}

    void tryGenerator(
        const std::unique_ptr<QueryGenerator>& queryGenerator,
        PatentReader& patentReader,
        SearchIndex& searchIndex,
        Searcher& searcher,
        const std::unique_ptr<Reporter>& reporter) {
        Timer timer;
        auto query = queryGenerator->generateQuery(targets, patentReader, searchIndex);
        double seconds = timer.elapsedSeconds();

        if (query.empty()) {
            reporter->reportGenerator(id, queryGenerator->getName(), 0.0, seconds);
            return;
        }

        auto results = searcher.search(query);
        double score = calculateScore(results);

        reporter->reportGenerator(id, queryGenerator->getName(), score, seconds);

        if (score > bestScore) {
            bestScore = score;
            bestQuery = query;
            bestQueryGenerator = queryGenerator->getName();
        }
    }

private:
    double calculateScore(const ankerl::unordered_dense::set<std::string>& results) const {
        // Adapted from https://www.kaggle.com/competitions/uspto-explainable-ai/discussion/499981#2791642
        double totalScore = 0.0;
        int found = 0;

        for (std::size_t i = 0; i < 50; ++i) {
            if (results.contains(targets[i])) {
                ++found;
            }

            totalScore += static_cast<double>(found) / static_cast<double>(i + 1);
        }

        return totalScore / 50.0;
    }
};

inline std::vector<Task> generateQueries(const std::filesystem::path& testDataFile, int maxRows = -1) {
    Timer globalTimer;

    spdlog::info("Creating patent reader");
    PatentReader patentReader;

    spdlog::info("Creating search index reader");
    SearchIndexReader searchIndexReader(getFullIndexDirectory());

    spdlog::info("Reading reversed patent ids");
    auto patentIdsReversed = searchIndexReader.readPatentIdsReversed();

    std::vector<Task> tasks;
    std::vector<std::string> targets;

    spdlog::info("Reading test data");
    readNeighbors(
        testDataFile,
        [&](const std::string& publicationNumber, const std::vector<std::string>& neighbors) {
            tasks.emplace_back(tasks.size(), publicationNumber, neighbors);

            targets.reserve(targets.size() + neighbors.size());
            targets.insert(targets.end(), neighbors.begin(), neighbors.end());
        },
        maxRows);

    spdlog::info("Creating query generators");
    auto queryGenerators = createQueryGenerators();

    BS::thread_pool threadPool;
    std::mutex mutex;

    spdlog::info("Initializing reporter");
    auto initReporter = createReporter(true);
    initReporter->init(threadPool.get_thread_count(), tasks.size(), queryGenerators);

    spdlog::info("Finding best queries for {} test data rows", tasks.size());
    ProgressBar progressBar(tasks.size(), "Processing tasks");

    double totalScore = 0.0;
    double tasksProcessed = 0.0;

    threadPool.detach_blocks(
        static_cast<std::size_t>(0),
        tasks.size(),
        [&](std::size_t start, std::size_t end) {
            PatentReader localPatentReader(patentReader);

            SearchIndex searchIndex(searchIndexReader);
            Searcher searcher(searchIndex, patentIdsReversed);

            auto reporter = createReporter();

            for (std::size_t i = start; i < end; ++i) {
                Timer localTimer;
                auto& task = tasks[i];

                for (const auto& queryGenerator : queryGenerators) {
                    task.tryGenerator(queryGenerator, localPatentReader, searchIndex, searcher, reporter);

                    // The submission notebook may run for up to 9 hours
                    // It's ran in an environment with 4 CPUs and there are 2,500 tasks to process
                    // This means there are 51.84 seconds per task when nothing else has to happen
                    // Limiting to 40 seconds per task should prevent any timeouts
                    if (localTimer.elapsedSeconds() >= 40) {
                        spdlog::warn("Task {} timed out after {}", task.id, queryGenerator->getName());
                        break;
                    }
                }

                reporter->reportTask(task.id, task.bestQueryGenerator, task.bestScore, localTimer.elapsedSeconds());
                searchIndex.clearCache();

                std::lock_guard lock(mutex);
                totalScore += task.bestScore;
                ++tasksProcessed;

                progressBar.setDescription(
                    fmt::format("Processing tasks (mean score: {:.3f})", totalScore / tasksProcessed));
                progressBar.update(1);
            }
        },
        threadPool.get_thread_count() * 5);

    threadPool.wait();

    spdlog::info("Mean score: {:.3f}", totalScore / tasksProcessed);
    spdlog::info("Total time taken: {:.3f} seconds", globalTimer.elapsedSeconds());
    return tasks;
}
