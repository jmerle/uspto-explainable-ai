#pragma once

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <fmt/format.h>
#include <min-max_heap/mmheap.h>

#include <uspto/index.h>
#include <uspto/patents.h>
#include <uspto/queries.h>

extern "C" {
#include <borgelt/fpgrowth/fpgrowth.h>
}

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
            double score = static_cast<double>(count) / searchIndex.selectivity(term);
            if (score > bestScore) {
                bestTerm = term;
                bestScore = score;
            }
        }

        return bestTerm;
    }
};

class FPGrowthQueryGenerator : public QueryGenerator {
    struct FPResult {
        std::vector<std::string> terms;
        int support = std::numeric_limits<int>::lowest();
        double score = std::numeric_limits<double>::max();

        FPResult() = default;

        FPResult(const std::vector<std::string>& terms, int support, double score)
            : terms(terms),
              support(support),
              score(score) {}

        bool operator<(const FPResult& other) const {
            if (score == other.score) {
                return support > other.support;
            }

            return score < other.score;
        }
    };

    struct FPState {
        ITEMBASE* ibase;
        const SearchIndex& searchIndex;

        std::vector<FPResult> results;
        std::size_t resultsSize;

        FPState(ITEMBASE* ibase, const SearchIndex& searchIndex)
            : ibase(ibase),
              searchIndex(searchIndex),
              results(1000),
              resultsSize(0) {}

        void report(const std::vector<std::string>& terms, int support) {
            double selectivity = 1.0;
            for (const auto& term : terms) {
                selectivity *= searchIndex.selectivity(term);
            }

            double score = static_cast<double>(support) * selectivity;

            FPResult result(terms, support, score);
            mmheap::heap_insert_circular(result, results.data(), resultsSize, results.size());
        }
    };

    TermCategory::TermCategory categories;
    int minSupport;
    int minGroupSize;
    int maxGroupSize;

public:
    FPGrowthQueryGenerator(TermCategory::TermCategory categories, int minSupport, int minGroupSize, int maxGroupSize)
        : categories(categories),
          minSupport(minSupport),
          minGroupSize(minGroupSize),
          maxGroupSize(maxGroupSize) {}

    std::string getName() const override {
        return fmt::format(
            "FPGrowthQueryGenerator(categories={}, minSupport={}, minGroupSize={}, maxGroupSize={})",
            TermCategory::toString(categories),
            minSupport,
            minGroupSize,
            maxGroupSize);
    }

    std::string generateQuery(
        const std::vector<std::string>& targets,
        PatentReader& patentReader,
        const SearchIndex& searchIndex) const override {
        std::vector<std::vector<std::string>> termGroups;
        termGroups.reserve(targets.size());

        ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::set<std::string>> termsByTarget;
        termsByTarget.reserve(targets.size());

        bool hasTerms = false;

        for (const auto& target : targets) {
            auto terms = patentReader.readTerms(target, categories);

            termGroups.emplace_back(terms);
            termsByTarget.emplace(target, ankerl::unordered_dense::set<std::string>(terms.begin(), terms.end()));

            hasTerms = hasTerms || !terms.empty();
        }

        if (!hasTerms) {
            return "";
        }

        auto results = runFPGrowth(termGroups, searchIndex);

        std::vector<std::pair<FPResult, ankerl::unordered_dense::set<std::string>>> groups;
        int querySize = 0;

        ankerl::unordered_dense::map<std::string, int> targetCoverage;
        int uncoveredTargets = 50;

        for (const auto& result : results) {
            int newQuerySize = querySize + result.terms.size() + (querySize == 0 ? 0 : 1);
            if (newQuerySize > 50) {
                break;
            }

            ankerl::unordered_dense::set<std::string> coveredTargets;
            bool coversOldTarget = false;

            for (const auto& target : targets) {
                bool covered = true;
                const auto& targetTerms = termsByTarget[target];

                for (const auto& term : result.terms) {
                    if (!targetTerms.contains(term)) {
                        covered = false;
                        break;
                    }
                }

                if (!covered) {
                    continue;
                }

                coveredTargets.emplace(target);

                if (targetCoverage[target] > 0) {
                    coversOldTarget = true;
                    break;
                }
            }

            if (coversOldTarget) {
                continue;
            }

            groups.emplace_back(result, coveredTargets);
            for (const auto& target : coveredTargets) {
                if (targetCoverage[target] == 0) {
                    --uncoveredTargets;
                }

                ++targetCoverage[target];
            }

            querySize = newQuerySize;

            if (uncoveredTargets == 0) {
                break;
            }
        }

        for (const auto& result : results) {
            int newQuerySize = querySize + result.terms.size() + (querySize == 0 ? 0 : 1);
            if (newQuerySize > 50) {
                break;
            }

            ankerl::unordered_dense::set<std::string> coveredTargets;
            for (const auto& target : targets) {
                bool covered = true;
                const auto& targetTerms = termsByTarget[target];

                for (const auto& term : result.terms) {
                    if (!targetTerms.contains(term)) {
                        covered = false;
                        break;
                    }
                }

                if (covered) {
                    coveredTargets.emplace(target);
                }
            }

            groups.emplace_back(result, coveredTargets);
            for (const auto& target : coveredTargets) {
                ++targetCoverage[target];
            }

            querySize = newQuerySize;
        }

        std::vector<std::vector<std::string>> orGroups;
        std::vector<std::vector<std::string>> xorGroups;

        for (const auto& [result, coveredTargets] : groups) {
            // The Whoosh query parser becomes a lot slower when there are many XOR operators
            if (xorGroups.size() == 5) {
                orGroups.emplace_back(result.terms);
                continue;
            }

            bool exclusive = true;
            for (const auto& target : coveredTargets) {
                if (targetCoverage[target] != 1) {
                    exclusive = false;
                    break;
                }
            }

            if (exclusive) {
                xorGroups.emplace_back(result.terms);
            } else {
                orGroups.emplace_back(result.terms);
            }
        }

        return serializeTermGroups(orGroups, xorGroups);
    }

private:
    std::vector<FPResult> runFPGrowth(
        const std::vector<std::vector<std::string>>& termGroups,
        const SearchIndex& searchIndex) const {
        auto* ibase = ib_create(0, 0);
        auto* tabag = tbg_create(ibase);

        for (std::size_t i = 0; i < 50; ++i) {
            for (const auto& term : termGroups[i]) {
                ib_add2ta(ibase, term.c_str());
            }

            ib_finta(ibase, 50 - i);
            tbg_add(tabag, nullptr);

            ib_clear(ibase);
        }

        auto* fpg = fpg_create(
            FPG_FREQUENT,
            minSupport,
            100.0,
            100.0,
            minGroupSize,
            maxGroupSize,
            FPG_LDRATIO,
            FPG_NONE,
            1.0,
            FPG_COMPLEX,
            FPG_DEFAULT);

        fpg_data(fpg, tabag, 0, 2);
        auto* report = isr_create(ibase);

        FPState state(ibase, searchIndex);
        isr_setrepo(
            report,
            [](isreport* rep, void* data) {
                auto* state = static_cast<FPState*>(data);

                std::vector<std::string> terms;
                terms.reserve(isr_cnt(rep));
                for (int i = 0; i < isr_cnt(rep); ++i) {
                    terms.emplace_back(ib_name(state->ibase, isr_itemx(rep, i)));
                }

                state->report(terms, isr_supp(rep));
            },
            &state);

        fpg_report(fpg, report);
        fpg_mine(fpg, 0, 0);
        fpg_delete(fpg, 1);

        std::vector<FPResult> results;
        results.reserve(state.resultsSize);
        while (state.resultsSize > 0) {
            results.emplace_back(mmheap::heap_remove_min(state.results.data(), state.resultsSize));
        }

        return results;
    }

    std::string serializeTermGroups(
        const std::vector<std::vector<std::string>>& orGroups,
        const std::vector<std::vector<std::string>>& xorGroups) const {
        auto orQuery = joinTermGroups("OR", orGroups);
        auto xorQuery = joinTermGroups("XOR", xorGroups);

        if (!orGroups.empty() && !xorGroups.empty()) {
            return fmt::format("({}) XOR {}", orQuery, xorQuery);
        }

        if (!orGroups.empty()) {
            return orQuery;
        }

        if (!xorGroups.empty()) {
            return xorQuery;
        }

        return "";
    }

    std::string joinTermGroups(const std::string& op, const std::vector<std::vector<std::string>>& groups) const {
        if (groups.empty()) {
            return "";
        }

        if (groups.size() == 1) {
            return serializeTermGroup(groups[0]);
        }

        std::string out = fmt::format("({} {} {})", serializeTermGroup(groups[0]), op, serializeTermGroup(groups[1]));
        for (std::size_t i = 2; i < groups.size(); ++i) {
            out = fmt::format("({} {} {})", out, op, serializeTermGroup(groups[i]));
        }

        return out;
    }

    std::string serializeTermGroup(const std::vector<std::string>& group) const {
        if (group.size() == 1) {
            return group[0];
        }

        return fmt::format("({})", fmt::join(group, " "));
    }
};

inline std::vector<std::unique_ptr<QueryGenerator>> createQueryGenerators() {
    std::vector<std::unique_ptr<QueryGenerator>> out;

    for (auto categories : std::vector<TermCategory::TermCategory>{
             TermCategory::Cpc,
             TermCategory::Title,
             TermCategory::Abstract,
             TermCategory::Cpc | TermCategory::Title,
             TermCategory::Cpc | TermCategory::Abstract,
             TermCategory::Title | TermCategory::Abstract,
             TermCategory::Cpc | TermCategory::Title | TermCategory::Abstract,
         }) {
        for (int minSupport = 4; minSupport <= 10; minSupport += 2) {
            out.emplace_back(std::make_unique<FPGrowthQueryGenerator>(categories, minSupport, 2, 2));
        }
    }

    for (auto category : std::vector<TermCategory::TermCategory>{
             TermCategory::Cpc,
             TermCategory::Title,
             TermCategory::Abstract,
             TermCategory::Claims,
             TermCategory::Description,
         }) {
        out.emplace_back(std::make_unique<SingleTermQueryGenerator>(category));
    }

    return out;
}
