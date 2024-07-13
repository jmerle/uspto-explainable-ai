#pragma once

#include <algorithm>
#include <any>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <antlr4-runtime.h>
#include <min-max_heap/mmheap.h>
#include <roaring/roaring.hh>

#include <uspto/index.h>
#include <uspto/whoosh/WhooshBaseListener.h>
#include <uspto/whoosh/WhooshLexer.h>
#include <uspto/whoosh/WhooshParser.h>
#include <uspto/whoosh/WhooshVisitor.h>

class Searcher {
    SearchIndex& searchIndex;

    const std::vector<std::string>& patentIdsReversed;

    ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::set<std::string>> cache;

    struct MatchCollector : whoosh::WhooshVisitor {
        Searcher& searcher;

        explicit MatchCollector(Searcher& searcher)
            : searcher(searcher) {}

        std::any visitTerm(whoosh::WhooshParser::TermContext* ctx) override {
            return searcher.searchIndex.getTermBitset(ctx->TOKEN(0)->toString() + ":" + ctx->TOKEN(1)->toString());
        }

        std::any visitTermExpr(whoosh::WhooshParser::TermExprContext* ctx) override {
            return visit(ctx->term());
        }

        std::any visitWrappedExpr(whoosh::WhooshParser::WrappedExprContext* ctx) override {
            return visit(ctx->expr());
        }

        std::any visitOrExpr(whoosh::WhooshParser::OrExprContext* ctx) override {
            return std::any_cast<roaring::Roaring>(visit(ctx->left))
                   | std::any_cast<roaring::Roaring>(visit(ctx->right));
        }

        std::any visitAndExpr(whoosh::WhooshParser::AndExprContext* ctx) override {
            return std::any_cast<roaring::Roaring>(visit(ctx->left))
                   & std::any_cast<roaring::Roaring>(visit(ctx->right));
        }

        std::any visitXorExpr(whoosh::WhooshParser::XorExprContext* ctx) override {
            return std::any_cast<roaring::Roaring>(visit(ctx->left))
                   ^ std::any_cast<roaring::Roaring>(visit(ctx->right));
        }

        std::any visitNotExpr(whoosh::WhooshParser::NotExprContext* ctx) override {
            auto bits = std::any_cast<roaring::Roaring>(visit(ctx->right));
            bits.flip(0, searcher.searchIndex.getPatentCount());
            return bits;
        }
    };

    struct TermCollector : whoosh::WhooshBaseListener {
        const Searcher& searcher;

        ankerl::unordered_dense::map<std::string, double> terms;

        explicit TermCollector(const Searcher& searcher)
            : searcher(searcher) {}

        void enterTerm(whoosh::WhooshParser::TermContext* ctx) override {
            auto term = ctx->TOKEN(0)->toString() + ":" + ctx->TOKEN(1)->toString();

            double patentCount = searcher.searchIndex.getPatentCount();
            double termFrequency = searcher.searchIndex.getTermCardinality(term);
            double idf = std::log(patentCount / (termFrequency + 1)) + 1;

            terms[term] += idf;
        }
    };

    struct SearchResult {
        std::uint32_t patentId = std::numeric_limits<std::uint32_t>::max();
        double tfIdf = std::numeric_limits<double>::lowest();

        SearchResult() = default;

        SearchResult(std::uint32_t patentId, double tfIdf)
            : patentId(patentId),
              tfIdf(tfIdf) {}

        bool operator<(const SearchResult& other) const {
            if (tfIdf == other.tfIdf) {
                return patentId < other.patentId;
            }

            return tfIdf > other.tfIdf;
        }
    };

public:
    Searcher(SearchIndex& searchIndex, const std::vector<std::string>& patentIdsReversed)
        : searchIndex(searchIndex),
          patentIdsReversed(patentIdsReversed) {}

    void clearCache() {
        cache.clear();
    }

    ankerl::unordered_dense::set<std::string> search(const std::string& query) {
        auto cacheIt = cache.find(query);
        if (cacheIt != cache.end()) {
            return cacheIt->second;
        }

        antlr4::ANTLRInputStream input(query);
        whoosh::WhooshLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        whoosh::WhooshParser parser(&tokens);

        antlr4::tree::ParseTree* tree = parser.expr();

        MatchCollector matchCollector(*this);
        auto bits = std::any_cast<roaring::Roaring>(matchCollector.visit(tree));
        auto bitsCardinality = bits.cardinality();

        // Queries with too many results are unlikely to be winners, and are expensive to sort
        if (bitsCardinality > 5000) {
            cache.emplace(query, ankerl::unordered_dense::set<std::string>());
            return {};
        }

        std::vector<std::uint32_t> matchingPatentIds;
        matchingPatentIds.reserve(bitsCardinality);
        for (auto id : bits) {
            matchingPatentIds.emplace_back(id);
        }

        if (matchingPatentIds.size() <= 50) {
            return idsToPublicationNumbers(query, matchingPatentIds);
        }

        TermCollector termCollector(*this);
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&termCollector, tree);

        std::vector<SearchResult> results(50);
        std::size_t resultsSize = 0;

        for (const auto& id : matchingPatentIds) {
            double tfIdf = 0;

            for (const auto& [term, idf] : termCollector.terms) {
                const auto& counts = searchIndex.getTermCounts(term);

                auto countsIt = counts.find(id);
                if (countsIt != counts.end()) {
                    tfIdf += static_cast<double>(countsIt->second) * idf;
                }
            }

            SearchResult result(id, tfIdf);
            mmheap::heap_insert_circular(result, results.data(), resultsSize, results.size());
        }

        std::vector<std::uint32_t> sortedPatentIds;
        sortedPatentIds.reserve(resultsSize);
        while (resultsSize > 0) {
            sortedPatentIds.emplace_back(mmheap::heap_remove_min(results.data(), resultsSize).patentId);
        }

        return idsToPublicationNumbers(query, sortedPatentIds);
    }

private:
    ankerl::unordered_dense::set<std::string> idsToPublicationNumbers(
        const std::string& query,
        const std::vector<std::uint32_t>& ids) {
        std::size_t size = std::min(static_cast<std::size_t>(50), ids.size());

        ankerl::unordered_dense::set<std::string> out;
        out.reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            out.emplace(patentIdsReversed[ids[i]]);
        }

        cache.emplace(query, out);
        return out;
    }
};
