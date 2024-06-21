#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <random>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <BS_thread_pool.hpp>
#include <fmt/format.h>
#include <roaring/roaring.hh>
#include <spdlog/spdlog.h>

#include <uspto/config.h>
#include <uspto/csv.h>
#include <uspto/files.h>
#include <uspto/patents.h>
#include <uspto/progress.h>
#include <uspto/queries.h>

struct SearchIndex {
    ankerl::unordered_dense::map<std::string, std::uint32_t> ids;
    ankerl::unordered_dense::map<std::string, roaring::Roaring> termBitsets;
    ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>> termCounts;

    double selectivity(const std::string& term) const {
        double cardinality = termBitsets.at(term).cardinality();
        double patentCount = ids.size();

        return cardinality / patentCount;
    }
};

inline ankerl::unordered_dense::set<std::string> getPublicationNumbers(
    const std::filesystem::path& testDataFile,
    int maxRows = -1,
    bool extendNeighbors = true) {
    ankerl::unordered_dense::set<std::string> publicationNumbers;

    spdlog::info("Reading test data");
    readNeighbors(
        testDataFile,
        [&](const std::string& publicationNumber, const std::vector<std::string>& neighbors) {
            publicationNumbers.insert(neighbors.begin(), neighbors.end());
        },
        maxRows);

    if (!extendNeighbors) {
        return publicationNumbers;
    }

    std::vector<std::string> allPublicationNumbers;
    spdlog::info("Reading nearest neighbors");
    readNeighbors(
        getCompetitionDataDirectory() / "nearest_neighbors.csv",
        [&](const std::string& publicationNumber, const std::vector<std::string>& neighbors) {
            allPublicationNumbers.emplace_back(publicationNumber);
        });

    std::shuffle(allPublicationNumbers.begin(), allPublicationNumbers.end(), std::mt19937{std::random_device{}()});

    for (std::size_t i = 0; i < allPublicationNumbers.size() && publicationNumbers.size() < 200'000; ++i) {
        publicationNumbers.emplace(allPublicationNumbers[i]);
    }

    return publicationNumbers;
}

inline SearchIndex getSearchIndex(
    PatentReader& patentReader,
    const std::filesystem::path& testDataFile,
    int maxRows = -1,
    bool extendPatents = true) {
    auto publicationNumbers = getPublicationNumbers(testDataFile, maxRows, extendPatents);

    std::vector<std::string> sortedPublicationNumbers(publicationNumbers.begin(), publicationNumbers.end());
    patentReader.sortToIndex(sortedPublicationNumbers);

    SearchIndex searchIndex;

    searchIndex.ids.reserve(sortedPublicationNumbers.size());
    for (const auto& publicationNumber : sortedPublicationNumbers) {
        searchIndex.ids.emplace(publicationNumber, searchIndex.ids.size());
    }

    ProgressBar progressBar(publicationNumbers.size(), fmt::format("Processing patents"));
    BS::thread_pool threadPool;
    std::mutex mutex;

    TermCategory::TermCategory allCategories =
            TermCategory::Cpc
            | TermCategory::Title
            | TermCategory::Abstract
            | TermCategory::Claims
            | TermCategory::Description;

    threadPool.detach_blocks(
        static_cast<std::size_t>(0),
        publicationNumbers.size(),
        [&](std::size_t start, std::size_t end) {
            PatentReader localPatentReader(patentReader);
            ankerl::unordered_dense::map<std::string, roaring::Roaring> localTermBitsets;
            ankerl::unordered_dense::map<
                std::string,
                ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>> localTermCounts;

            for (std::size_t i = start; i < end; ++i) {
                const auto& publicationNumber = sortedPublicationNumbers[i];
                auto id = searchIndex.ids.at(publicationNumber);

                auto patentTerms = localPatentReader.readTermsWithCounts(publicationNumber, allCategories);
                for (const auto& [term, count] : patentTerms) {
                    localTermBitsets[term].add(id);
                    localTermCounts[term][id] = count;
                }
            }

            progressBar.update(end - start);

            std::lock_guard lock(mutex);
            for (const auto& [term, bitset] : localTermBitsets) {
                searchIndex.termBitsets[term] |= bitset;
            }

            for (const auto& [term, counts] : localTermCounts) {
                auto& existingCounts = searchIndex.termCounts[term];
                for (const auto& [id, count] : counts) {
                    existingCounts[id] = count;
                }
            }
        },
        threadPool.get_thread_count() * 3);

    threadPool.wait();

    for (auto& [_, bitset] : searchIndex.termBitsets) {
        bitset.runOptimize();
        bitset.shrinkToFit();
    }

    return searchIndex;
}
