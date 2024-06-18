#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <fmt/format.h>

#include <uspto/index.h>
#include <uspto/patents.h>
#include <uspto/queries.h>

class QueryGenerator {
public:
    virtual ~QueryGenerator() = default;

    virtual std::string getName() const = 0;

    virtual std::string generateQuery(
        const std::vector<std::string>& targets,
        PatentReader& patentReader,
        const SearchIndex& searchIndex) const = 0;
};

class SingleTermQueryGenerator : public QueryGenerator {
    TermCategory::TermCategory category;

public:
    explicit SingleTermQueryGenerator(TermCategory::TermCategory category)
        : category(category) {}

    std::string getName() const override {
        return fmt::format("SingleTermQueryGenerator(category={})", TermCategory::toString(category));
    }

    std::string generateQuery(
        const std::vector<std::string>& targets,
        PatentReader& patentReader,
        const SearchIndex& searchIndex) const override {
        ankerl::unordered_dense::map<std::string, int> counts;

        for (const auto& target : targets) {
            for (const auto& term : patentReader.readTerms(target, category)) {
                ++counts[term];
            }
        }

        std::string bestTerm = "";
        double bestScore = 0.0;

        for (const auto& [term, count] : counts) {
            double score =
                    static_cast<double>(count) / static_cast<double>(searchIndex.termBitsets.at(term).cardinality());

            if (score > bestScore) {
                bestTerm = term;
                bestScore = score;
            }
        }

        return bestTerm;
    }
};

inline std::vector<std::unique_ptr<QueryGenerator>> createQueryGenerators() {
    std::vector<std::unique_ptr<QueryGenerator>> out;

    std::vector<TermCategory::TermCategory> uniqueCategories{
        TermCategory::Cpc,
        TermCategory::Title,
        TermCategory::Abstract,
        TermCategory::Claims,
        TermCategory::Description,
    };

    std::vector<TermCategory::TermCategory> combinedCategories{
        TermCategory::Title | TermCategory::Abstract,
        TermCategory::Title | TermCategory::Abstract | TermCategory::Claims,
        TermCategory::Title | TermCategory::Abstract | TermCategory::Claims | TermCategory::Description,
        TermCategory::Abstract | TermCategory::Claims | TermCategory::Description,
        TermCategory::Claims | TermCategory::Description,
        TermCategory::Cpc
        | TermCategory::Title
        | TermCategory::Abstract
        | TermCategory::Claims
        | TermCategory::Description,
    };

    for (const auto& category : uniqueCategories) {
        out.emplace_back(std::make_unique<SingleTermQueryGenerator>(category));
    }

    return out;
}
