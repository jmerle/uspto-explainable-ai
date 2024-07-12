#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <BS_thread_pool.hpp>
#include <fmt/format.h>
#include <roaring/roaring.hh>
#include <spdlog/spdlog.h>

#include <uspto/files.h>
#include <uspto/patents.h>
#include <uspto/progress.h>
#include <uspto/queries.h>

class SearchIndexReader : public DataReader<std::uint16_t> {
public:
    using DataReader::DataReader;

    std::uint32_t readPatentCount() {
        seekToKey("ids");
        return readScalar<std::uint32_t>();
    }

    std::vector<std::string> readPatentIdsReversed() {
        seekToKey("ids");

        auto size = readScalar<std::uint32_t>();
        std::vector<std::string> publicationNumbers(size);

        for (std::uint32_t i = 0; i < size; ++i) {
            auto publicationNumber = readString<std::uint8_t>();
            auto id = readScalar<std::uint32_t>();

            publicationNumbers[id] = publicationNumber;
        }

        return publicationNumbers;
    }

    roaring::Roaring readTermBitset(const std::string& term) {
        seekToKey(term);

        auto size = readScalar<std::uint32_t>();
        auto buffer = readRaw(size);

        return roaring::Roaring::read(buffer.data(), false);
    }

    ankerl::unordered_dense::map<std::uint32_t, std::uint16_t> readTermCounts(const std::string& term) {
        seekToKey(" " + term);

        ankerl::unordered_dense::map<std::uint32_t, std::uint16_t> counts;

        auto size = readScalar<std::uint32_t>();
        counts.reserve(size);

        for (std::uint32_t i = 0; i < size; ++i) {
            auto patentId = readScalar<std::uint32_t>();
            auto count = readScalar<std::uint16_t>();

            counts.emplace(patentId, count);
        }

        return counts;
    }

    std::uint32_t readTermCardinality(const std::string& term) {
        seekToKey(" " + term);
        return readScalar<std::uint32_t>();
    }
};

class SearchIndexWriter : public DataWriter<std::uint16_t> {
public:
    using DataWriter::DataWriter;

    void writeIds(const ankerl::unordered_dense::map<std::string, std::uint32_t>& ids) {
        addKey("ids");

        writeScalar<std::uint32_t>(ids.size());
        for (const auto& [publicationNumber, id] : ids) {
            writeString<std::uint8_t>(publicationNumber);
            writeScalar<std::uint32_t>(id);
        }
    }

    void writeCounts(
        const std::string& term,
        const ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>& counts) {
        writeCountsAsBitset(term, counts);
        writeCountsAsMap(term, counts);
    }

private:
    void writeCountsAsBitset(
        const std::string& term,
        const ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>& counts) {
        roaring::Roaring bitset;
        roaring::BulkContext bulkContext;

        for (const auto& [patentId, _] : counts) {
            bitset.addBulk(bulkContext, patentId);
        }

        bitset.runOptimize();
        bitset.shrinkToFit();

        addKey(term);

        auto size = bitset.getSizeInBytes(false);
        std::vector<char> buffer(size);

        bitset.write(buffer.data(), false);

        writeScalar<std::uint32_t>(size);
        writeRaw(buffer.data(), size);
    }

    void writeCountsAsMap(
        const std::string& term,
        const ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>& counts) {
        addKey(" " + term);

        writeScalar<std::uint32_t>(counts.size());
        for (const auto& [patentId, count] : counts) {
            writeScalar<std::uint32_t>(patentId);
            writeScalar<std::uint16_t>(count);
        }
    }
};

class SearchIndex {
    SearchIndexReader reader;

    std::uint32_t patentCount;

    ankerl::unordered_dense::map<std::string, roaring::Roaring> bitsets;
    ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>> counts;
    ankerl::unordered_dense::map<std::string, std::uint32_t> cardinalities;

    std::vector<double> tfIdfScores;

public:
    explicit SearchIndex(const SearchIndexReader& reader)
        : reader(reader), patentCount(this->reader.readPatentCount()), tfIdfScores(patentCount) {}

    void clearCache() {
        bitsets.clear();
        counts.clear();
        cardinalities.clear();
    }

    std::uint32_t getPatentCount() const {
        return patentCount;
    }

    const roaring::Roaring& getTermBitset(const std::string& term) {
        auto it = bitsets.find(term);
        if (it != bitsets.end()) {
            return it->second;
        }

        bitsets.emplace(term, reader.readTermBitset(term));
        return bitsets[term];
    }

    const ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>& getTermCounts(const std::string& term) {
        auto it = counts.find(term);
        if (it != counts.end()) {
            return it->second;
        }

        counts.emplace(term, reader.readTermCounts(term));
        return counts[term];
    }

    std::uint32_t getTermCardinality(const std::string& term) {
        auto it = cardinalities.find(term);
        if (it != cardinalities.end()) {
            return it->second;
        }

        cardinalities.emplace(term, reader.readTermCardinality(term));
        return cardinalities[term];
    }

    double getTermSelectivity(const std::string& term) {
        return static_cast<double>(getTermCardinality(term)) / static_cast<double>(patentCount);
    }
};

inline void processTermGroup(
    SearchIndexWriter& writer,
    const PatentReader& patentReader,
    BS::thread_pool& threadPool,
    const std::vector<std::string>& publicationNumbers,
    const ankerl::unordered_dense::map<std::string, std::uint32_t>& patentIds,
    TermCategory::TermCategory category,
    const ankerl::unordered_dense::set<std::string>& terms,
    const std::string& description) {
    ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>> termCounts;
    std::mutex termCountsMutex;

    ProgressBar progressBar(publicationNumbers.size(), description);
    bool acceptAllTerms = terms.empty();

    threadPool.detach_blocks(
        static_cast<std::size_t>(0),
        publicationNumbers.size(),
        [&](std::size_t start, std::size_t end) {
            PatentReader localPatentReader(patentReader);
            ankerl::unordered_dense::map<
                std::string,
                ankerl::unordered_dense::map<std::uint32_t, std::uint16_t>> localTermCounts;

            for (std::size_t i = start; i < end; ++i) {
                const auto& publicationNumber = publicationNumbers[i];
                auto patentId = patentIds.at(publicationNumber);

                auto patentCounts = localPatentReader.readTermsWithCounts(publicationNumber, category);
                for (const auto& [term, count] : patentCounts) {
                    if (acceptAllTerms || terms.contains(term)) {
                        localTermCounts[term].emplace(patentId, count);
                    }
                }
            }

            progressBar.update(end - start);

            std::lock_guard lock(termCountsMutex);
            for (const auto& [term, counts] : localTermCounts) {
                termCounts[term].insert(counts.begin(), counts.end());
            }
        },
        threadPool.get_thread_count() * 5);

    threadPool.wait();

    for (const auto& [term, counts] : termCounts) {
        writer.writeCounts(term, counts);
    }
}

inline void processTerms(
    SearchIndexWriter& writer,
    const PatentReader& patentReader,
    BS::thread_pool& threadPool,
    const std::vector<std::string>& publicationNumbers,
    const ankerl::unordered_dense::map<std::string, std::uint32_t>& patentIds,
    TermCategory::TermCategory category) {
    if (category != TermCategory::Claims && category != TermCategory::Description) {
        processTermGroup(
            writer,
            patentReader,
            threadPool,
            publicationNumbers,
            patentIds,
            category,
            {},
            fmt::format("Processing {} terms", TermCategory::toString(category)));
        return;
    }

    ankerl::unordered_dense::map<std::string, std::uint32_t> termCounts;
    std::mutex termCountsMutex;

    ProgressBar progressBar(
        publicationNumbers.size(),
        fmt::format("Collecting {} terms", TermCategory::toString(category)));

    threadPool.detach_blocks(
        static_cast<std::size_t>(0),
        publicationNumbers.size(),
        [&](std::size_t start, std::size_t end) {
            PatentReader localPatentReader(patentReader);
            ankerl::unordered_dense::map<std::string, std::uint32_t> localTermCounts;

            for (std::size_t i = start; i < end; ++i) {
                auto patentTermCounts = localPatentReader.readTermsWithCounts(publicationNumbers[i], category);
                for (const auto& [term, count] : patentTermCounts) {
                    localTermCounts[term] += count;
                }
            }

            progressBar.update(end - start);

            std::lock_guard lock(termCountsMutex);
            for (const auto& [term, count] : localTermCounts) {
                termCounts[term] += count;
            }
        },
        threadPool.get_thread_count() * 5);

    threadPool.wait();

    int groupCount = category == TermCategory::Claims ? 5 : 20;
    spdlog::info(
        "Processing {} {} terms in {} groups",
        termCounts.size(),
        TermCategory::toString(category),
        groupCount);

    std::vector<std::pair<std::string, std::uint32_t>> termCountsSorted(termCounts.begin(), termCounts.end());
    std::sort(
        termCountsSorted.begin(),
        termCountsSorted.end(),
        [](const std::pair<std::string, std::uint32_t>& a, const std::pair<std::string, std::uint32_t>& b) {
            return a.second < b.second;
        });

    termCounts.clear();

    for (int i = 0; i < groupCount; ++i) {
        ankerl::unordered_dense::set<std::string> terms;
        terms.reserve(termCountsSorted.size() / groupCount);
        for (std::size_t j = i; j < termCountsSorted.size(); j += groupCount) {
            terms.emplace(termCountsSorted[j].first);
        }

        processTermGroup(
            writer,
            patentReader,
            threadPool,
            publicationNumbers,
            patentIds,
            category,
            terms,
            fmt::format("Processing {} terms (group {}/{})", TermCategory::toString(category), i + 1, groupCount));
    }
}

inline void createSearchIndex(
    const ankerl::unordered_dense::set<std::string>& publicationNumbers,
    const std::filesystem::path& outputDirectory,
    const PatentReader& patentReader,
    bool includeDescription) {
    spdlog::info(
        "Building search index containing {} patents in {}",
        publicationNumbers.size(),
        outputDirectory.c_str());

    spdlog::info("Sorting publication numbers");
    std::vector<std::string> sortedPublicationNumbers(publicationNumbers.begin(), publicationNumbers.end());
    patentReader.sortToIndex(sortedPublicationNumbers);

    SearchIndexWriter searchIndexWriter(outputDirectory);

    ankerl::unordered_dense::map<std::string, std::uint32_t> patentIds;
    patentIds.reserve(sortedPublicationNumbers.size());

    for (const auto& publicationNumber : sortedPublicationNumbers) {
        patentIds.emplace(publicationNumber, patentIds.size());
    }

    spdlog::info("Saving ids");
    searchIndexWriter.writeIds(patentIds);

    BS::thread_pool threadPool;

    for (const auto category : std::vector<TermCategory::TermCategory>{
             TermCategory::Cpc,
             TermCategory::Title,
             TermCategory::Abstract,
             TermCategory::Claims,
             TermCategory::Description,
         }) {
        if (!includeDescription && category == TermCategory::Description) {
            continue;
        }

        processTerms(searchIndexWriter, patentReader, threadPool, sortedPublicationNumbers, patentIds, category);
    }
}
