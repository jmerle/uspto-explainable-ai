#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <fmt/format.h>
#include <min-max_heap/mmheap.h>

#include <uspto/index.h>
#include <uspto/patents.h>
#include <uspto/queries.h>
#include <uspto/searcher.h>
#include <uspto/timer.h>

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
        SearchIndex& searchIndex,
        Searcher& searcher) const = 0;

    double getQueryScore(Searcher& searcher, const std::string& query, const std::vector<std::string>& targets) const {
        if (query.empty()) {
            return 0;
        }

        auto results = searcher.search(query);

        // Adapted from https://www.kaggle.com/competitions/uspto-explainable-ai/discussion/499981#2791642
        double totalScore = 0.0;
        int found = 0;

        for (std::size_t i = 0; i < targets.size(); ++i) {
            if (results.contains(targets[i])) {
                ++found;
            }

            totalScore += static_cast<double>(found) / static_cast<double>(i + 1);
        }

        return totalScore / static_cast<double>(targets.size());
    }

protected:
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
        SearchIndex& searchIndex,
        Searcher& searcher) const override {
        ankerl::unordered_dense::map<std::string, int> counts;

        for (const auto& target : targets) {
            for (const auto& term : patentReader.readTerms(target, category)) {
                ++counts[term];
            }
        }

        std::string bestTerm = "";
        double bestScore = 0.0;

        for (const auto& [term, count] : counts) {
            double score = static_cast<double>(count) / searchIndex.getTermSelectivity(term);
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
        SearchIndex& searchIndex;

        std::vector<FPResult> results;
        std::size_t resultsSize;

        FPState(ITEMBASE* ibase, SearchIndex& searchIndex)
            : ibase(ibase),
              searchIndex(searchIndex),
              results(1000),
              resultsSize(0) {}

        void report(const std::vector<std::string>& terms, int support) {
            double selectivity = searchIndex.getTermSelectivity(terms[0]);
            for (std::size_t i = 1; i < terms.size(); ++i) {
                selectivity *= searchIndex.getTermSelectivity(terms[i]);
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
        SearchIndex& searchIndex,
        Searcher& searcher) const override {
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
                continue;
            }

            ankerl::unordered_dense::set<std::string> coveredTargets;
            bool coversNewTarget = false;

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
                coversNewTarget = coversNewTarget || targetCoverage[target] == 0;
            }

            if (uncoveredTargets > 0 && !coversNewTarget) {
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
        SearchIndex& searchIndex) const {
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
};

class OptimizingQueryGenerator : public QueryGenerator {
    TermCategory::TermCategory categories;

    struct Group {
        std::size_t id;
        std::vector<std::size_t> targets;
        std::vector<std::pair<std::string, double>> availableTerms;

        std::vector<std::string> selectedTerms;
        double selectivity = 1.0;

        Group(
            std::size_t id,
            const std::vector<std::size_t>& targets,
            const std::vector<std::pair<std::string, double>>& availableTerms)
            : id(id),
              targets(targets),
              availableTerms(availableTerms) {}
    };

    struct Action {
        virtual ~Action() = default;

        virtual void apply(std::vector<std::vector<std::size_t>>& targetGroups) = 0;
    };

    struct SplitToNewGroupAction : Action {
        std::size_t groupIdx;
        std::size_t targetIdx;

        SplitToNewGroupAction(std::size_t groupIdx, std::size_t targetIdx)
            : groupIdx(groupIdx),
              targetIdx(targetIdx) {}

        void apply(std::vector<std::vector<std::size_t>>& targetGroups) override {
            targetGroups.emplace_back(std::vector<std::size_t>({targetGroups[groupIdx][targetIdx]}));
            auto& group = targetGroups[groupIdx];
            group.erase(group.begin() + targetIdx);
        }
    };

    struct MoveToNewGroupAction : Action {
        std::size_t groupIdx1;
        std::size_t groupIdx2;
        std::size_t targetIdx;

        MoveToNewGroupAction(std::size_t groupIdx1, std::size_t groupIdx2, std::size_t targetIdx)
            : groupIdx1(groupIdx1),
              groupIdx2(groupIdx2),
              targetIdx(targetIdx) {}

        void apply(std::vector<std::vector<std::size_t>>& targetGroups) override {
            auto& oldGroup = targetGroups[groupIdx1];
            auto& newGroup = targetGroups[groupIdx2];

            newGroup.emplace_back(oldGroup[targetIdx]);
            oldGroup.erase(oldGroup.begin() + targetIdx);
        }
    };

    struct MergeGroupsAction : Action {
        std::size_t groupIdx1;
        std::size_t groupIdx2;

        MergeGroupsAction(std::size_t groupIdx1, std::size_t groupIdx2)
            : groupIdx1(groupIdx1),
              groupIdx2(groupIdx2) {}

        void apply(std::vector<std::vector<std::size_t>>& targetGroups) override {
            auto& group1 = targetGroups[groupIdx1];
            const auto& group2 = targetGroups[groupIdx2];

            group1.insert(group1.end(), group2.begin(), group2.end());
            targetGroups.erase(targetGroups.begin() + groupIdx2);
        }
    };

public:
    explicit OptimizingQueryGenerator(TermCategory::TermCategory categories)
        : categories(categories) {}

    std::string getName() const override {
        return fmt::format("OptimizingQueryGenerator(categories={})", TermCategory::toString(categories));
    }

    std::string generateQuery(
        const std::vector<std::string>& targets,
        PatentReader& patentReader,
        SearchIndex& searchIndex,
        Searcher& searcher) const override {
        std::vector<std::set<std::string>> termsByTarget;
        termsByTarget.reserve(targets.size());
        for (const auto& target : targets) {
            auto terms = patentReader.readTerms(target, categories);
            termsByTarget.emplace_back(terms.begin(), terms.end());
        }

        std::vector<std::vector<std::size_t>> targetGroups(1);
        for (std::size_t i = 0; i < targets.size(); ++i) {
            targetGroups[0].emplace_back(i);
        }

        std::string bestQuery = createQuery(targetGroups, termsByTarget, searchIndex);
        auto maxScore = getQueryScore(searcher, bestQuery, targets);

        Timer timer;
        while (timer.elapsedSeconds() < 20) {
            bool foundImprovement = false;

            for (const auto& action : getActions(targetGroups)) {
                auto newTargetGroups = targetGroups;
                action->apply(newTargetGroups);

                auto query = createQuery(newTargetGroups, termsByTarget, searchIndex);
                auto score = getQueryScore(searcher, query, targets);

                if (score > maxScore) {
                    bestQuery = query;
                    maxScore = score;
                    targetGroups = newTargetGroups;
                    foundImprovement = true;
                    break;
                }
            }

            if (!foundImprovement) {
                break;
            }
        }

        return bestQuery;
    }

private:
    std::vector<std::unique_ptr<Action>> getActions(const std::vector<std::vector<std::size_t>>& targetGroups) const {
        std::vector<std::unique_ptr<Action>> actions;

        for (std::size_t groupIdx1 = 0; groupIdx1 < targetGroups.size(); ++groupIdx1) {
            for (std::size_t groupIdx2 = groupIdx1 + 1; groupIdx2 < targetGroups.size(); ++groupIdx2) {
                actions.emplace_back(std::make_unique<MergeGroupsAction>(groupIdx1, groupIdx2));
            }

            const auto& group1 = targetGroups[groupIdx1];
            if (group1.size() == 1) {
                continue;
            }

            for (std::size_t targetIdx = 0; targetIdx < group1.size(); ++targetIdx) {
                actions.emplace_back(std::make_unique<SplitToNewGroupAction>(groupIdx1, targetIdx));

                for (std::size_t groupIdx2 = 0; groupIdx2 < targetGroups.size(); ++groupIdx2) {
                    if (groupIdx1 != groupIdx2) {
                        actions.emplace_back(
                            std::make_unique<MoveToNewGroupAction>(groupIdx1, groupIdx2, targetIdx));
                    }
                }
            }
        }

        return actions;
    }

    std::string createQuery(
        const std::vector<std::vector<std::size_t>>& targetGroups,
        const std::vector<std::set<std::string>>& termsByTarget,
        SearchIndex& searchIndex) const {
        std::vector<Group> groups;
        groups.reserve(targetGroups.size());

        for (const auto& targetGroup : targetGroups) {
            auto sharedTerms = termsByTarget[targetGroup[0]];

            for (std::size_t i = 1; i < targetGroup.size(); ++i) {
                const auto& targetTerms = termsByTarget[targetGroup[i]];

                std::set<std::string> intersection;
                std::set_intersection(
                    sharedTerms.begin(),
                    sharedTerms.end(),
                    targetTerms.begin(),
                    targetTerms.end(),
                    std::inserter(intersection, intersection.begin()));

                sharedTerms = intersection;
            }

            std::vector<std::pair<std::string, double>> availableTerms;

            for (const auto& term : sharedTerms) {
                double selectivity = searchIndex.getTermSelectivity(term);
                if (selectivity < 0.01) {
                    availableTerms.emplace_back(term, selectivity);
                }
            }

            std::sort(
                availableTerms.begin(),
                availableTerms.end(),
                [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
                    return a.second > b.second;
                });

            groups.emplace_back(groups.size(), targetGroup, availableTerms);
        }

        int tokensRemaining = 50;
        while (tokensRemaining > 0) {
            int bestGroup = -1;
            std::size_t minTermCount = 100;
            double maxSelectivity = -1;
            int bestGroupRequiredTokens = -1;

            for (int i = 0; i < groups.size(); ++i) {
                const auto& group = groups[i];

                int requiredTokens = 1;
                if (tokensRemaining < 50 && group.selectedTerms.empty()) {
                    requiredTokens = 2;
                }

                if (requiredTokens > tokensRemaining) {
                    continue;
                }

                if (group.availableTerms.empty()) {
                    continue;
                }

                std::size_t termCount = group.selectedTerms.size();
                double selectivity = group.selectivity;

                if (termCount < minTermCount || (termCount == minTermCount && selectivity > maxSelectivity)) {
                    bestGroup = i;
                    minTermCount = termCount;
                    maxSelectivity = selectivity;
                    bestGroupRequiredTokens = requiredTokens;
                }
            }

            if (bestGroup == -1) {
                break;
            }

            auto& group = groups[bestGroup];

            const auto& newTerm = group.availableTerms.back();
            group.selectedTerms.emplace_back(newTerm.first);
            group.selectivity *= newTerm.second;

            group.availableTerms.pop_back();
            tokensRemaining -= bestGroupRequiredTokens;
        }

        std::sort(
            groups.begin(),
            groups.end(),
            [](const Group& a, const Group& b) {
                return a.selectivity > b.selectivity;
            });

        std::vector<std::vector<std::string>> orGroups;
        std::vector<std::vector<std::string>> xorGroups;

        for (const auto& group : groups) {
            if (group.selectedTerms.empty()) {
                continue;
            }

            // The Whoosh query parser becomes a lot slower when there are many XOR operators
            if (xorGroups.size() == 5) {
                orGroups.emplace_back(group.selectedTerms);
                continue;
            }

            bool exclusive = true;
            for (const auto& otherGroup : groups) {
                if (group.id == otherGroup.id) {
                    continue;
                }

                for (const auto& target : otherGroup.targets) {
                    bool containsAll = true;

                    const auto& targetTerms = termsByTarget[target];
                    for (const auto& term : group.selectedTerms) {
                        if (targetTerms.find(term) == targetTerms.end()) {
                            containsAll = false;
                            break;
                        }
                    }

                    if (containsAll) {
                        exclusive = false;
                        break;
                    }
                }

                if (!exclusive) {
                    break;
                }
            }

            if (exclusive) {
                xorGroups.emplace_back(group.selectedTerms);
            } else {
                orGroups.emplace_back(group.selectedTerms);
            }
        }

        return serializeTermGroups(orGroups, xorGroups);
    }
};

class BestEffortQueryGenerator : public QueryGenerator {
    TermCategory::TermCategory categories;

public:
    explicit BestEffortQueryGenerator(TermCategory::TermCategory categories)
        : categories(categories) {}

    std::string getName() const override {
        return fmt::format("BestEffortQueryGenerator(categories={})", TermCategory::toString(categories));
    }

    std::string generateQuery(
        const std::vector<std::string>& targets,
        PatentReader& patentReader,
        SearchIndex& searchIndex,
        Searcher& searcher) const override {
        ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::set<std::string>> termsByTarget;
        termsByTarget.reserve(targets.size());

        ankerl::unordered_dense::map<std::string, std::vector<std::string>> sortedTermsByTarget;
        sortedTermsByTarget.reserve(targets.size());

        for (const auto& target : targets) {
            auto terms = patentReader.readTerms(target, categories);

            std::sort(
                terms.begin(),
                terms.end(),
                [&](const std::string& a, const std::string& b) {
                    return searchIndex.getTermCardinality(a) < searchIndex.getTermCardinality(b);
                });

            termsByTarget.emplace(target, ankerl::unordered_dense::set<std::string>(terms.begin(), terms.end()));
            sortedTermsByTarget.emplace(target, terms);
        }

        std::string bestQuery;
        double maxScore = 0.0;

        std::vector<ankerl::unordered_dense::set<std::string>> groups;
        int remainingTokens = 50;

        std::string previousTarget;
        ankerl::unordered_dense::set<std::string> skippedTargets;

        while (remainingTokens > 0) {
            auto query = createQuery(groups, termsByTarget);

            auto score = getQueryScore(searcher, query, targets);
            if (score > maxScore) {
                bestQuery = query;
                maxScore = score;
            }

            std::string currentTarget;

            if (!groups.empty() && groups.back().size() < 2) {
                currentTarget = previousTarget;
            } else {
                auto results = !query.empty() ? searcher.search(query) : ankerl::unordered_dense::set<std::string>();
                for (const auto& target : targets) {
                    if (!skippedTargets.contains(target) && !results.contains(target)) {
                        currentTarget = target;
                        break;
                    }
                }
            }

            if (currentTarget.empty()) {
                break;
            }

            int requiredTokens = 1;
            if (!groups.empty() && currentTarget != previousTarget) {
                ++requiredTokens;
            }

            if (requiredTokens > remainingTokens) {
                break;
            }

            const auto& availableTerms = sortedTermsByTarget[currentTarget];
            std::string bestTerm;

            if (currentTarget != previousTarget) {
                if (!availableTerms.empty()) {
                    bestTerm = availableTerms[0];
                }
            } else {
                const auto& currentGroup = groups.back();
                std::vector<std::string> currentGroupVec(currentGroup.begin(), currentGroup.end());

                auto bitset = searchIndex.getTermBitset(currentGroupVec[0]);
                for (std::size_t i = 1; i < currentGroup.size(); ++i) {
                    bitset &= searchIndex.getTermBitset(currentGroupVec[i]);
                }

                std::size_t minCardinality = std::numeric_limits<std::size_t>::max();

                for (const auto& term : availableTerms) {
                    if (currentGroup.contains(term)) {
                        continue;
                    }

                    std::size_t cardinality = bitset.and_cardinality(searchIndex.getTermBitset(term));
                    if (cardinality < minCardinality) {
                        bestTerm = term;
                        minCardinality = cardinality;
                    }
                }
            }

            if (bestTerm.empty()) {
                skippedTargets.emplace(currentTarget);
                continue;
            }

            if (currentTarget != previousTarget) {
                groups.emplace_back();
            }

            groups.back().emplace(bestTerm);

            remainingTokens -= requiredTokens;
            previousTarget = currentTarget;
        }

        return bestQuery;
    }

private:
    std::string createQuery(
        const std::vector<ankerl::unordered_dense::set<std::string>>& groups,
        const ankerl::unordered_dense::map<
            std::string,
            ankerl::unordered_dense::set<std::string>>& termsByTarget) const {
        std::vector<std::vector<std::string>> targetsByGroup;
        ankerl::unordered_dense::map<std::string, std::size_t> groupsByTarget;

        for (const auto& groupTerms : groups) {
            std::vector<std::string> targets;

            for (const auto& [target, targetTerms] : termsByTarget) {
                bool matches = true;
                for (const auto& term : groupTerms) {
                    if (!targetTerms.contains(term)) {
                        matches = false;
                        break;
                    }
                }

                if (matches) {
                    targets.emplace_back(target);
                    ++groupsByTarget[target];
                }
            }

            targetsByGroup.emplace_back(targets);
        }

        std::vector<std::vector<std::string>> orGroups;
        std::vector<std::vector<std::string>> xorGroups;

        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto& group = groups[i];

            // The Whoosh query parser becomes a lot slower when there are many XOR operators
            if (xorGroups.size() == 5) {
                orGroups.emplace_back(group.begin(), group.end());
                continue;
            }

            bool isExclusive = true;
            for (const auto& target : targetsByGroup[i]) {
                if (groupsByTarget[target] > 1) {
                    isExclusive = false;
                    break;
                }
            }

            if (isExclusive) {
                xorGroups.emplace_back(group.begin(), group.end());
            } else {
                orGroups.emplace_back(group.begin(), group.end());
            }
        }

        return serializeTermGroups(orGroups, xorGroups);
    }
};

inline std::vector<std::unique_ptr<QueryGenerator>> createQueryGenerators() {
    std::vector<std::unique_ptr<QueryGenerator>> out;

    out.emplace_back(
        std::make_unique<BestEffortQueryGenerator>(
            TermCategory::Cpc | TermCategory::Title | TermCategory::Abstract | TermCategory::Claims));

    out.emplace_back(
        std::make_unique<OptimizingQueryGenerator>(
            TermCategory::Cpc | TermCategory::Title | TermCategory::Abstract | TermCategory::Claims));

    for (auto categories : std::vector<TermCategory::TermCategory>{
             TermCategory::Cpc,
             TermCategory::Title,
             TermCategory::Abstract,
             TermCategory::Cpc | TermCategory::Title,
             TermCategory::Cpc | TermCategory::Abstract,
             TermCategory::Title | TermCategory::Abstract,
             TermCategory::Cpc | TermCategory::Title | TermCategory::Abstract,
         }) {
        out.emplace_back(std::make_unique<FPGrowthQueryGenerator>(categories, 2, 2, 2));
    }

    for (auto category : std::vector<TermCategory::TermCategory>{
             TermCategory::Cpc,
             TermCategory::Title,
             TermCategory::Abstract,
             TermCategory::Claims,
         }) {
        out.emplace_back(std::make_unique<SingleTermQueryGenerator>(category));
    }

    return out;
}
